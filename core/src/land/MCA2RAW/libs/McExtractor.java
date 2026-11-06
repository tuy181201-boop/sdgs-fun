import net.querz.nbt.io.NBTDeserializer;
import net.querz.nbt.io.NamedTag;
import net.querz.nbt.tag.*;
import java.io.*;
import java.nio.file.*;
import java.text.SimpleDateFormat;
import java.util.*;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import java.util.zip.*;

public class McExtractor {
    private static final Set<String> IGNORE = new HashSet<>(Arrays.asList(
        "minecraft:air", "minecraft:cave_air", "minecraft:void_air", "minecraft:water", 
        "minecraft:lava", "minecraft:short_grass", "minecraft:tall_grass", "minecraft:oak_leaves", 
        "minecraft:spruce_leaves", "minecraft:birch_leaves", "minecraft:snow", "minecraft:ice", "minecraft:vine"
    ));
    private static final Pattern REGION_PATTERN = Pattern.compile("r\\.(-?\\d+)\\.(-?\\d+)\\.mca");

    public static void main(String[] args) {
        if (args.length < 2) {
            System.out.println("ERROR: Usage: java -jar McExtractor.jar <region_folder> <output_folder>");
            return;
        }

        File regionFolder = new File(args[0]);
        File outputFolder = new File(args[1]);
        if (!outputFolder.exists()) outputFolder.mkdirs();

        File[] files = regionFolder.listFiles((dir, name) -> name.endsWith(".mca"));
        if (files == null || files.length == 0) {
            System.out.println("ERROR: No .mca files found.");
            return;
        }
        Arrays.sort(files);

        // 1. ВЫВОД ТАБЛИЦЫ
        System.out.println("TABLE_START");
        System.out.printf("%-15s | %-10s | %-20s%n", "Region File", "Size (MB)", "Last Modified");
        System.out.println("----------------+------------+---------------------");
        SimpleDateFormat sdf = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
        
        for (File file : files) {
            long sizeMb = file.length() / (1024 * 1024);
            String date = sdf.format(new Date(file.lastModified()));
            System.out.printf("%-15s | %-10d | %-20s%n", file.getName(), sizeMb, date);
        }
        System.out.println("TABLE_END");

        // 2. КОНВЕРТАЦИЯ
        int total = files.length;
        for (int i = 0; i < total; i++) {
            File file = files[i];
            System.out.println("PROGRESS:" + (int)((i / (double)total) * 100));
            try {
                processRegion(file, outputFolder);
            } catch (Exception e) {
                System.out.println("ERROR in " + file.getName() + ": " + e.getMessage());
            }
        }
        System.out.println("PROGRESS:100");
        System.out.println("SUCCESS");
    }

    private static void processRegion(File file, File outDir) throws IOException {
        Matcher m = REGION_PATTERN.matcher(file.getName());
        if (!m.matches()) return;
        
        int regX = Integer.parseInt(m.group(1));
        int regZ = Integer.parseInt(m.group(2));
        File rawFile = new File(outDir, "t." + regX + "." + regZ + ".raw");

        short[] heightMap = new short[512 * 512];
        Arrays.fill(heightMap, (short) -64);

        try (RandomAccessFile raf = new RandomAccessFile(file, "r")) {
            byte[] locations = new byte[4096];
            raf.readFully(locations);
            raf.seek(4096);
            NBTDeserializer deserializer = new NBTDeserializer(false);

            for (int i = 0; i < 1024; i++) {
                int offset = ((locations[i * 4] & 0xFF) << 16) | ((locations[i * 4 + 1] & 0xFF) << 8) | (locations[i * 4 + 2] & 0xFF);
                if (offset == 0) continue;
                raf.seek((long) offset * 4096);
                int length = raf.readInt();
                int comp = raf.readByte() & 0xFF;
                if (length <= 1) continue;
                byte[] chunkData = new byte[length - 1];
                raf.readFully(chunkData);
                InputStream is = getDecompressedStream(chunkData, comp);
                if (is == null) continue;
                NamedTag tag = deserializer.fromStream(is);
                parseChunkToMap((CompoundTag) tag.getTag(), i % 32, i / 32, heightMap);
            }
        }
        try (DataOutputStream dos = new DataOutputStream(new BufferedOutputStream(new FileOutputStream(rawFile)))) {
            for (short h : heightMap) dos.writeShort(h);
        }
    }

    private static void parseChunkToMap(CompoundTag chunk, int lcx, int lcz, short[] globalMap) {
        ListTag<CompoundTag> sections = (ListTag<CompoundTag>) chunk.getListTag("sections");
        if (sections == null) return;
        int[] chunkHeights = new int[256];
        Arrays.fill(chunkHeights, -9999);
        for (CompoundTag section : sections) {
            byte y = section.getByte("Y");
            CompoundTag bs = section.getCompoundTag("block_states");
            if (bs == null) continue;
            ListTag<CompoundTag> palette = (ListTag<CompoundTag>) bs.getListTag("palette");
            if (palette == null) continue;
            long[] data = bs.getLongArrayTag("data") != null ? bs.getLongArrayTag("data").getValue() : null;
            int bits = Math.max(4, 32 - Integer.numberOfLeadingZeros(palette.size() - 1));
            int[] blocks = unpackLongs(data, bits, 4096);
            for (int z = 0; z < 16; z++) {
                for (int x = 0; x < 16; x++) {
                    for (int yl = 15; yl >= 0; yl--) {
                        int idx = yl * 256 + z * 16 + x;
                        int id = (blocks != null && idx < blocks.length) ? blocks[idx] : 0;
                        String name = (id >= 0 && id < palette.size()) ? palette.get(id).getString("Name") : "minecraft:air";
                        if (!IGNORE.contains(name)) {
                            int wy = (y * 16) + yl;
                            if (wy > chunkHeights[z * 16 + x]) chunkHeights[z * 16 + x] = wy;
                            break;
                        }
                    }
                }
            }
        }
        for (int x = 0; x < 16; x++) {
            for (int z = 0; z < 16; z++) {
                if (chunkHeights[z * 16 + x] != -9999)
                    globalMap[((lcz * 16 + z) * 512) + (lcx * 16 + x)] = (short) chunkHeights[z * 16 + x];
            }
        }
    }

    private static InputStream getDecompressedStream(byte[] data, int type) throws IOException {
        if (type == 1) return new GZIPInputStream(new ByteArrayInputStream(data));
        if (type == 2) return new InflaterInputStream(new ByteArrayInputStream(data));
        if (type == 3) return new ByteArrayInputStream(data);
        return null;
    }

    private static int[] unpackLongs(long[] longs, int bits, int size) {
        if (longs == null) return null;
        int[] result = new int[size];
        int perLong = 64 / bits;
        long mask = (1L << bits) - 1;
        int index = 0;
        for (long l : longs) {
            for (int k = 0; k < perLong && index < size; k++) result[index++] = (int) ((l >> (k * bits)) & mask);
        }
        return result;
    }
}