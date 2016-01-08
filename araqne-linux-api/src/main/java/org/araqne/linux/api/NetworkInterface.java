/*
 * Copyright 2011 Future Systems
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 * http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package org.araqne.linux.api;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.Inet6Address;
import java.net.InetAddress;
import java.util.ArrayList;
import java.util.List;

// ifconfig
public class NetworkInterface {
	private String name;
	private String linkEncap;
	private String hwaddr;
	private InetAddress inet;
	private InetAddress ptp;
	private InetAddress mask;
	private Inet6Address inet6;
	private int cidr;
	private String scope;
	private String options;
	private int mtu;
	private int metric;
	private RxPacket rxPacket;
	private TxPacket txPacket;
	private long rxBytes;
	private long txBytes;
	private int interrupt;
	private String baseAddress;
	private String memory;

	public String getName() {
		return name;
	}

	public String getLinkEncap() {
		return linkEncap;
	}

	public String getHwaddr() {
		return hwaddr;
	}

	public InetAddress getInet() {
		return inet;
	}

	public InetAddress getPtp() {
		return ptp;
	}

	public InetAddress getMask() {
		return mask;
	}

	public Inet6Address getInet6() {
		return inet6;
	}

	public int getCidr() {
		return cidr;
	}

	public String getScope() {
		return scope;
	}

	public String getOptions() {
		return options;
	}

	public int getMtu() {
		return mtu;
	}

	public int getMetric() {
		return metric;
	}

	public RxPacket getRxPacket() {
		return rxPacket;
	}

	public TxPacket getTxPacket() {
		return txPacket;
	}

	public long getRxBytes() {
		return rxBytes;
	}

	public long getTxBytes() {
		return txBytes;
	}

	public int getInterrupt() {
		return interrupt;
	}

	public String getBaseAddress() {
		return baseAddress;
	}

	public String getMemory() {
		return memory;
	}

	public static List<NetworkInterface> getNetworkInterfaces() throws IOException {
		List<NetworkInterface> ifaces = new ArrayList<NetworkInterface>();
		java.lang.Process p = null;
		BufferedReader br = null;

		try {
			p = Runtime.getRuntime().exec("ifconfig -a");
			br = new BufferedReader(new InputStreamReader(p.getInputStream()));

			while (true) {
				NetworkInterface iface = parse(br);
				if (iface == null)
					break;
				ifaces.add(iface);
			}
		} finally {
			if (p != null)
				p.destroy();
			if (br != null)
				br.close();
		}

		return ifaces;
	}

	public static NetworkInterface parse(BufferedReader br) throws IOException {
		NetworkInterface iface = null;
		String regex = "\t| {2,}";
		String line = null;

		while ((line = br.readLine()) != null && !line.trim().equals("")) {
			if (iface == null)
				iface = new NetworkInterface();
			line = line.trim();

			if (iface.name == null && !line.replace("\t", " ").startsWith(" ")) {
				String name = line.trim().replace("\t", " ").substring(0, line.trim().replace("\t", " ").indexOf(" ")).trim();
				if (name.endsWith(":"))
					name = name.substring(0, name.length() - 1);
				iface.name = name;
			}

			if (line.startsWith("RX")) {
				if (iface.rxPacket == null)
					iface.rxPacket = new RxPacket();
				if (line.contains("packets"))
					iface.rxPacket.packets = Long.parseLong(getValue(line, "packets"));
				if (line.contains("bytes"))
					iface.rxBytes = Long.parseLong(getValue(line, "bytes"));
				if (line.contains("errors"))
					iface.rxPacket.errors = Long.parseLong(getValue(line, "errors"));
				if (line.contains("dropped"))
					iface.rxPacket.dropped = Long.parseLong(getValue(line, "dropped"));
				if (line.contains("overruns"))
					iface.rxPacket.overruns = Long.parseLong(getValue(line, "overruns"));
				if (line.contains("frame"))
					iface.rxPacket.frame = Long.parseLong(getValue(line, "frame"));
				if (line.contains("TX bytes")) {
					iface.txBytes = Long.parseLong(getValue(line, "TX bytes"));
				}
			} else if (line.startsWith("TX")) {
				if (iface.txPacket == null)
					iface.txPacket = new TxPacket();

				if (line.contains("packets"))
					iface.txPacket.packets = Long.parseLong(getValue(line, "packets"));
				if (line.contains("bytes"))
					iface.txBytes = Long.parseLong(getValue(line, "bytes"));
				if (line.contains("errors"))
					iface.txPacket.errors = Long.parseLong(getValue(line, "errors"));
				if (line.contains("dropped"))
					iface.txPacket.dropped = Long.parseLong(getValue(line, "dropped"));
				if (line.contains("overruns"))
					iface.txPacket.overruns = Long.parseLong(getValue(line, "overruns"));
				if (line.contains("carrier"))
					iface.txPacket.carrier = Long.parseLong(getValue(line, "carrier"));
				if (line.contains("collisions"))
					iface.txPacket.collisions = Long.parseLong(getValue(line, "collisions"));

			} else if (line.contains("carrier"))
				iface.txPacket.carrier = Long.parseLong(getValue(line, "carrier"));
			else if (line.contains("collisions")) {
				iface.txPacket.collisions = Long.parseLong(getValue(line, "collisions"));
				if (line.contains("txqueuelen"))
					iface.txPacket.queuelen = Long.parseLong(getValue(line, "txqueuelen"));
			} else if (line.contains("txqueuelen")) {
				if (iface.txPacket == null)
					iface.txPacket = new TxPacket();
				iface.txPacket.queuelen = Long.parseLong(getValue(line, "txqueuelen"));
				iface.linkEncap = line.substring(line.indexOf("(") + 1, line.indexOf(")")).toLowerCase();
				int s = line.indexOf(" ") + 1;
				iface.hwaddr = line.substring(s, line.indexOf(" ", s));
			} else {
				String[] tokens = line.trim().split(regex);
				for (String token : tokens) {
					token = token.toLowerCase().trim();
					if (token.startsWith("link encap")) {
						iface.linkEncap = getValue(token, "link encap");
					} else if (token.startsWith("mtu")) {
						iface.mtu = Integer.parseInt(getValue(token, "mtu"));
					} else if (token.startsWith("inet addr")) {
						iface.inet = InetAddress.getByName(getValue(token, "inet addr"));
					} else if (token.startsWith("inet6 addr")) {
						String value = getValue(token, "inet6 addr");
						iface.inet6 = (Inet6Address) Inet6Address.getByName(value.substring(0, value.indexOf("/")));
						iface.cidr = Integer.parseInt(value.substring(value.indexOf("/") + 1));
						iface.scope = token.substring(token.lastIndexOf(":") + 1).toLowerCase();
					} else if (token.startsWith("inet ")) {
						iface.inet = InetAddress.getByName(getValue(token, "inet"));
					} else if (token.startsWith("inet6 ")) {
						iface.inet6 = (Inet6Address) Inet6Address.getByName(getValue(token, "inet6"));
					} else if (token.startsWith("prefixlen")) {
						iface.cidr = Integer.parseInt(getValue(token, "prefixlen"));
					} else if (token.startsWith("scopeid")) {
						String scope = getValue(token, "scopeid");
						iface.scope = scope.substring(scope.indexOf("<") + 1, scope.length() - 1).toLowerCase();
					} else if (token.startsWith("p-t-p")) {
						iface.mask = InetAddress.getByName(getValue(token, "p-t-p"));
					} else if (token.startsWith("mask")) {
						iface.mask = InetAddress.getByName(getValue(token, "mask"));
					} else if (token.startsWith("netmask")) {
						iface.mask = InetAddress.getByName(getValue(token, "netmask"));
					} else if (token.startsWith("metric")) {
						iface.metric = Integer.parseInt(getValue(token, "metric"));
					} else if (token.startsWith("hwaddr")) {
						iface.hwaddr = getValue(token, "hwaddr");
					} else if (token.matches("[A-Z ]+"))
						iface.options = token;
					else if (token.contains(" ")) {
						if (token.startsWith("HWaddr "))
							iface.hwaddr = token.substring("HWaddr ".length());
					}
				}
			}
		}

		return iface;
	}

	private static String getValue(String line, String name) {
		int s = line.indexOf(name) + name.length() + 1;
		if (name.equals("inet6 addr"))
			s += 1;
		int e = line.indexOf(" ", s);

		if (e == -1)
			return line.substring(s).trim();
		return line.substring(s, e).trim();
	}

	public static class RxPacket {
		private long packets;
		private long errors;
		private long dropped;
		private long overruns;
		private long frame;

		public RxPacket() {
		}

		public long getPackets() {
			return packets;
		}

		public long getErrors() {
			return errors;
		}

		public long getDropped() {
			return dropped;
		}

		public long getOverruns() {
			return overruns;
		}

		public long getFrame() {
			return frame;
		}

		@Override
		public String toString() {
			return "RxPacket [packets=" + packets + ", errors=" + errors + ", dropped=" + dropped + ", overruns=" + overruns
					+ ", frame=" + frame + "]";
		}
	}

	public static class TxPacket {
		private long packets;
		private long errors;
		private long dropped;
		private long overruns;
		private long carrier;
		private long collisions;
		private long queuelen;

		public TxPacket() {
		}

		public long getPackets() {
			return packets;
		}

		public long getErrors() {
			return errors;
		}

		public long getDropped() {
			return dropped;
		}

		public long getOverruns() {
			return overruns;
		}

		public long getCarrier() {
			return carrier;
		}

		public long getCollisions() {
			return collisions;
		}

		public long getQueuelen() {
			return queuelen;
		}

		@Override
		public String toString() {
			return "TxPacket [packets=" + packets + ", errors=" + errors + ", dropped=" + dropped + ", overruns=" + overruns
					+ ", carrier=" + carrier + ", collisions=" + collisions + ", queuelen=" + queuelen + "]";
		}
	}
}
