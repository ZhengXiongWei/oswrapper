package org.araqne.linux.api;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.nio.charset.Charset;
import java.util.ArrayList;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

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

	@Override
	public String toString() {
		return "name=" + name + ", used=" + used + ", available=" + available + ", path=" + path;
	}

	public static List<DiskPartition> getDiskPartitions() {
		return getDiskPartitions(false);
	}

	public static List<DiskPartition> getDiskPartitions(boolean localOnly) {
		List<String> lines = readPartitions(localOnly);
		return parseDiskPartitions(lines);
	}

	public static List<DiskPartition> parseDiskPartitions(List<String> lines) {
		List<DiskPartition> stats = new ArrayList<DiskPartition>();

		List<String> headers = parseHeader(lines.get(0));
		int columnCount = headers.size();

		for (int index = 1; index < lines.size(); index++) {
			String line = lines.get(index);
			List<String> parts = parseLine(line);
			while (parts.size() < columnCount) {
				line += " " + lines.get(++index);
				parts = parseLine(line);
			}

			DiskPartition p = parseDiskStat(headers, parts);
			stats.add(p);
		}
		return stats;
	}

	public static List<String> parseLine(String line) {
		List<String> l = new ArrayList<String>();

		Matcher m = Pattern.compile("\\s\\d+\\s").matcher(line);
		if (!m.find()) {
			return l;
		}

		String name = line.substring(0, m.start()).trim();

		String s = line.substring(m.start()).trim();
		int pathBegin = s.lastIndexOf('%');
		String path = null;
		if (pathBegin > 0) {
			path = s.substring(pathBegin + 1).trim();
			s = s.substring(0, pathBegin).trim();
		}

		// first space containing path
		l.add(name);

		String[] tokens = s.split("[ ]+");
		for (String t : tokens)
			l.add(t);

		// last space containing path
		if (path != null)
			l.add(path);

		return l;
	}

	private static List<String> parseHeader(String s) {
		String[] tokens = s.split("[ ]+");
		List<String> headers = new ArrayList<String>();

		boolean mountToken = false;
		for (String token : tokens) {
			if (token.equals("Mounted")) {
				headers.add("Mounted on");
				mountToken = true;
			} else if (mountToken && token.equals("on")) {
				continue;
			} else {
				headers.add(token);
			}
		}

		return headers;
	}

	private static List<String> readPartitions(boolean localOnly) {
		java.lang.Process p = null;
		BufferedReader br = null;
		List<String> lines = new ArrayList<String>();
		try {
			p = Runtime.getRuntime().exec(localOnly ? "df -l" : "df");

			br = new BufferedReader(new InputStreamReader(p.getInputStream(), Charset.forName("utf-8")));
			String line = null;
			while ((line = br.readLine()) != null) {
				lines.add(line);
			}
		} catch (IOException e) {
		} finally {
			if (p != null)
				p.destroy();
			if (br != null)
				try {
					br.close();
				} catch (IOException e) {
				}
		}
		return lines;
	}

	private static DiskPartition parseDiskStat(List<String> headers, List<String> parts) {
		int nameIndex = getHeaderIndex(headers, "Filesystem");
		int usedIndex = getHeaderIndex(headers, "Used");
		int availableIndex = getHeaderIndex(headers, "Available");
		int mountedOnIndex = getHeaderIndex(headers, "Mounted on");

		int unit = 1024;
		for (String h : headers) {
			if (h.equals("1K-blocks")) {
				unit = 1024;
				break;
			} else if (h.equals("512-blocks")) {
				unit = 512;
				break;
			}
		}

		DiskPartition stat = new DiskPartition();
		stat.setName(parts.get(nameIndex));
		stat.setUsed(Long.valueOf(parts.get(usedIndex)) * unit);
		stat.setAvailable(Long.valueOf(parts.get(availableIndex)) * unit);
		stat.setPath(parts.get(mountedOnIndex));
		return stat;
	}

	private static int getHeaderIndex(List<String> headers, String name) {
		int i = 0;
		for (String h : headers) {
			if (h.equals(name))
				return i;

			i++;
		}

		throw new IllegalStateException("header not found, expected '" + name + "'");
	}
}
