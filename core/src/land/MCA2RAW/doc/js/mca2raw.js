// --- Логика копирования кода ---
document.querySelectorAll('.copy-btn').forEach(button => {
    button.addEventListener('click', () => {
        const codeBlock = button.nextElementSibling.querySelector('code');
        const textToCopy = codeBlock.innerText;

        navigator.clipboard.writeText(textToCopy).then(() => {
            const originalText = button.innerText;
            button.innerText = 'Скопировано!';
            button.style.background = 'var(--green)';
            button.style.color = 'var(--bg-base)';

            setTimeout(() => {
                button.innerText = originalText;
                button.style.background = '';
                button.style.color = '';
            }, 2000);
        });
    });
});

// --- Подсветка активного пункта меню при скролле ---
const sections = document.querySelectorAll('section');
const navLinks = document.querySelectorAll('.nav-links a');

const observerOptions = {
    root: null,
    rootMargin: '0px',
    threshold: 0.3
};

const observer = new IntersectionObserver((entries) => {
    entries.forEach(entry => {
        if (entry.isIntersecting) {
            navLinks.forEach(link => {
                link.classList.remove('active');
                if (link.getAttribute('href') === `#${entry.target.id}`) {
                    link.classList.add('active');
                }
            });
        }
    });
}, observerOptions);

sections.forEach(section => {
    observer.observe(section);
});