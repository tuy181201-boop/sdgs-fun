#include "network_module.h"
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/ip/tcp.hpp>
namespace sdgs {
namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace net = boost::asio;
using tcp = net::ip::tcp;
void NetworkModule::init(const std::string& worldPath)
{
	std::cout << "[Network] WebSocket server starting on port 8080" << std::endl;
	std::thread([this]()
	{
		try
		{
			net::io_context ioc;
			tcp::acceptor acceptor(ioc, tcp::endpoint(tcp::v4(), 8080));
			while (true)
			{
				tcp::socket socket(ioc);
				acceptor.accept(socket);
				std::thread([s = std::move(socket)]() mutable
				{
					try
					{
						websocket::stream<tcp::socket> ws(std::move(s));
						ws.accept();
						beast::flat_buffer buffer;
						while (true)
						{
							ws.read(buffer);
							ws.text(ws.got_text());
							ws.write(buffer.data());
							buffer.consume(buffer.size());
						}
					}
					catch (const std::exception& e)
					{
						std::cerr << "[Network] WS error: " << e.what() << std::endl;
					}
				}).detach();
			}
		}
		catch (const std::exception& e)
		{
			std::cerr << "[Network] Server error: " << e.what() << std::endl;
		}
	}).detach();
}
void NetworkModule::update() {}
void NetworkModule::shutdown() { std::cout << "[Network] Shutdown" << std::endl; }
}