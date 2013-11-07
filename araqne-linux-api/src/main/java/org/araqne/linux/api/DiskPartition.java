package org.araqne.linux.api;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.List;

public class DiskPartition {
	private String name;
	private long used;
	private long available;
	private String path;

	public String getName() {
		return name;
	}

	public long getTotal() {
		return this.used + this.available;
	}

	public void setName(String name) {
		this.name = name;
	}

	public long getUsed() {
		return used;
	}

	public void setUsed(long used) {
		this.used = used;
	}

	public long getAvailable() {
		return available;
	}

	public void setAvailable(long available) {
		this.available = available;
	}

	public String getPath() {
		return path;
	}

	public void setPath(String path) {
		this.path = path;
	}

	public static List<DiskPartition> getDiskPartitions() {
		List<DiskPartition> stats = new ArrayList<DiskPartition>();
		java.lang.Process p = null;
		BufferedReader br = null;
		try {
			p = Runtime.getRuntime().exec("df");
			// TODO charset
			br = new BufferedReader(new InputStreamReader(p.getInputStream()));
			br.readLine();
			String line = null;
			while ((line = br.readLine()) != null) {
				stats.add(getDiskStat(line));
			}
		} catch (IOException e) {
			e.printStackTrace();
		} finally {
			if (p != null)
				p.destroy();
			if (br != null)
				try {
					br.close();
				} catch (IOException e) {
				}
		}
		return stats;
	}

	public static DiskPartition getDiskStat(String line) {
		String[] parts = line.split("[ ]+");
		DiskPartition stat = new DiskPartition();
		stat.setName(parts[0]);
		stat.setUsed(Long.valueOf(parts[2]));
		stat.setAvailable(Long.valueOf(parts[3]));
		stat.setPath(parts[5]);
		return stat;
	}
}
