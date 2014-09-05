package org.araqne.linux.api;

import static org.junit.Assert.assertEquals;

import java.util.ArrayList;
import java.util.List;

import org.junit.Test;

public class DiskPartitionTest {

	@Test
	public void getDiskPartition1() {
		List<String> lines = new ArrayList<String>();
		lines.add("Filesystem           1K-blocks      Used Available Use% Mounted on");
		lines.add("/dev/sda1        9156984 5075680   3609496  59% /");

		List<DiskPartition> partitions = DiskPartition.parseDiskPartitions(lines);
		assertEquals(1, partitions.size());
		verify(partitions.get(0), "/dev/sda1", 5075680l * 1024, 3609496l * 1024, "/");
	}

	@Test
	public void getDiskPartition2() {
		List<String> lines = new ArrayList<String>();
		lines.add("Filesystem           1K-blocks      Used Available Use% Mounted on");
		lines.add("/dev/mapper/vg_linux-lv_root");
		lines.add("51609340  45357008");
		lines.add("5728100  89% /");

		List<DiskPartition> partitions = DiskPartition.parseDiskPartitions(lines);
		assertEquals(1, partitions.size());
		verify(partitions.get(0), "/dev/mapper/vg_linux-lv_root", 45357008l * 1024, 5728100l * 1024, "/");
	}

	@Test
	public void testLinuxDiskPartitions() {
		List<String> lines = new ArrayList<String>();
		lines.add("Filesystem           1K-blocks      Used Available Use% Mounted on");
		lines.add("/dev/mapper/VolGroup00-LogVol00");
		lines.add("                      16124848   3787816  11504728  25% /");
		lines.add("/dev/xvda1              101086     21870     73997  23% /boot");
		lines.add("tmpfs                   524288         0    524288   0% /dev/shm");
		lines.add("/dev/xvdb1            82567188  44004080  34368940  57% /home");

		List<DiskPartition> partitions = DiskPartition.parseDiskPartitions(lines);
		assertEquals(4, partitions.size());
		verify(partitions.get(0), "/dev/mapper/VolGroup00-LogVol00", 3787816 * 1024L, 11504728 * 1024L, "/");
		verify(partitions.get(1), "/dev/xvda1", 21870 * 1024, 73997 * 1024, "/boot");
		verify(partitions.get(2), "tmpfs", 0, 524288 * 1024, "/dev/shm");
	}

	@Test
	public void testOsxDiskPartitions() {
		List<String> lines = new ArrayList<String>();
		lines.add("Filesystem                        512-blocks      Used Available Capacity  iused    ifree %iused  Mounted on");
		lines.add("/dev/disk0s2                       389057912 349665128  38880784    90% 43772139  4860098   90%   /");
		lines.add("devfs                                    379       379         0   100%      656        0  100%   /dev");
		lines.add("/dev/disk0s4                        78123000  50762336  27360664    65%   157817 13827411    1%   /Volumes/BOOTCAMP");
		lines.add("map -hosts                                 0         0         0   100%        0        0  100%   /net");
		lines.add("map auto_home                              0         0         0   100%        0        0  100%   /home");
		lines.add("localhost:/AAr1EGYFFuA61rgZQOMlgm  389057912 389057912         0   100%        0        0  100%   /Volumes/MobileBackups");
		lines.add("/dev/disk2s2                          415680    347288     68392    84%    43409     8549   84%   /Volumes/MacUpdate 2013 Freebie Bundle");

		List<DiskPartition> partitions = DiskPartition.parseDiskPartitions(lines);
		assertEquals(7, partitions.size());

		verify(partitions.get(0), "/dev/disk0s2", 349665128L * 512, 38880784L * 512, "/");
		verify(partitions.get(1), "devfs", 379 * 512, 0, "/dev");
		verify(partitions.get(2), "/dev/disk0s4", 50762336L * 512, 27360664L * 512, "/Volumes/BOOTCAMP");
		verify(partitions.get(3), "map -hosts", 0, 0, "/net");
		verify(partitions.get(4), "map auto_home", 0, 0, "/home");
		verify(partitions.get(5), "localhost:/AAr1EGYFFuA61rgZQOMlgm", 389057912L * 512, 0, "/Volumes/MobileBackups");
		verify(partitions.get(6), "/dev/disk2s2", 347288L * 512, 68392L * 512, "/Volumes/MacUpdate 2013 Freebie Bundle");
	}

	private void verify(DiskPartition p, String name, long used, long available, String path) {
		assertEquals(name, p.getName());
		assertEquals(used, p.getUsed());
		assertEquals(available, p.getAvailable());
		assertEquals(path, p.getPath());
	}
}
