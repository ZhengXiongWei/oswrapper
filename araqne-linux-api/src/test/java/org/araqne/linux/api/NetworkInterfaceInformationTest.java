package org.araqne.linux.api;

import static org.junit.Assert.assertEquals;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.util.List;

import org.araqne.linux.api.NetworkInterface.RxPacket;
import org.araqne.linux.api.NetworkInterface.TxPacket;
import org.junit.Assert;
import org.junit.Test;

public class NetworkInterfaceInformationTest {
	@Test
	public void getNetworklInterfaceInformationTest() {
		List<NetworkInterfaceInformation> informations = null;

		try {
			informations = NetworkInterfaceInformation
					.getNetworkInterfaceInformations(new File("src/test/resources/NetworkInterfaces"));
		} catch (IOException e) {
			Assert.fail("cannot not found dev file");
		}
		for (NetworkInterfaceInformation nii : informations) {
			if (nii.getName().equals("lo")) {
				assertEquals(1557111, nii.getRxBytes());
				assertEquals(8191, nii.getRxPackets());
			} else if (nii.getName().equals("eth0")) {
				assertEquals(2768360, nii.getRxBytes());
				assertEquals(45390, nii.getRxPackets());
				assertEquals(258966074, nii.getTxBytes());
				assertEquals(87851, nii.getTxPackets());
			}
		}
	}

	@Test
	public void testCentos6NetworkInterfaceParse() {
		BufferedReader br = null;
		try {
			br = new BufferedReader(new FileReader(new File("src/test/resources/ifconfig1")));
			NetworkInterface nic = NetworkInterface.parse(br);

			assertEquals("eth0", nic.getName());
			assertEquals("ethernet", nic.getLinkEncap());
			assertEquals("08:00:27:02:A7:9A".toLowerCase(), nic.getHwaddr());
			assertEquals("172.20.1.26", nic.getInet().getHostAddress());
			assertEquals(null, nic.getPtp());
			assertEquals("255.255.255.0", nic.getMask().getHostAddress());
			assertEquals("fe80:0:0:0:a00:27ff:fe02:a79a", nic.getInet6().getHostAddress());
			assertEquals(64, nic.getCidr());
			assertEquals("link", nic.getScope());
			assertEquals(1500, nic.getMtu());
			assertEquals(1, nic.getMetric());
			assertEquals(178586, nic.getRxBytes());
			assertEquals(200252, nic.getTxBytes());
			RxPacket rxPacket = nic.getRxPacket();
			assertEquals(1543, rxPacket.getPackets());
			assertEquals(1, rxPacket.getErrors());
			assertEquals(2, rxPacket.getDropped());
			assertEquals(3, rxPacket.getOverruns());
			assertEquals(4, rxPacket.getFrame());
			TxPacket txPacket = nic.getTxPacket();
			assertEquals(1012, txPacket.getPackets());
			assertEquals(5, txPacket.getErrors());
			assertEquals(6, txPacket.getDropped());
			assertEquals(7, txPacket.getOverruns());
			assertEquals(8, txPacket.getCarrier());
			assertEquals(9, txPacket.getCollisions());
			assertEquals(1000, txPacket.getQueuelen());
			assertEquals(0, nic.getInterrupt());
			assertEquals(null, nic.getBaseAddress());
			assertEquals(null, nic.getMemory());
		} catch (Exception e) {
			Assert.fail("cannot not parse ifconfig");
		} finally {
			if (br != null)
				try {
					br.close();
				} catch (IOException e) {
				}
		}
	}

	@Test
	public void testCentos7NetworkInterfaceParse() {
		BufferedReader br = null;
		try {
			br = new BufferedReader(new FileReader(new File("src/test/resources/ifconfig2")));
			NetworkInterface nic = NetworkInterface.parse(br);

			assertEquals("enp4s0", nic.getName());
			assertEquals("ethernet", nic.getLinkEncap());
			assertEquals("10:c3:7b:93:0b:bf".toLowerCase(), nic.getHwaddr());
			assertEquals("172.20.20.21", nic.getInet().getHostAddress());
			assertEquals(null, nic.getPtp());
			assertEquals("255.255.255.0", nic.getMask().getHostAddress());
			assertEquals("fe80:0:0:0:12c3:7bff:fe93:bbf", nic.getInet6().getHostAddress());
			assertEquals(64, nic.getCidr());
			assertEquals("link", nic.getScope());
			assertEquals(1500, nic.getMtu());
			assertEquals(0, nic.getMetric());
			assertEquals(156889050, nic.getRxBytes());
			assertEquals(6062445, nic.getTxBytes());
			RxPacket rxPacket = nic.getRxPacket();
			assertEquals(181374, rxPacket.getPackets());
			assertEquals(1, rxPacket.getErrors());
			assertEquals(2, rxPacket.getDropped());
			assertEquals(3, rxPacket.getOverruns());
			assertEquals(4, rxPacket.getFrame());
			TxPacket txPacket = nic.getTxPacket();
			assertEquals(95235, txPacket.getPackets());
			assertEquals(5, txPacket.getErrors());
			assertEquals(6, txPacket.getDropped());
			assertEquals(7, txPacket.getOverruns());
			assertEquals(8, txPacket.getCarrier());
			assertEquals(9, txPacket.getCollisions());
			assertEquals(1000, txPacket.getQueuelen());
			assertEquals(0, nic.getInterrupt());
			assertEquals(null, nic.getBaseAddress());
			assertEquals(null, nic.getMemory());
		} catch (Exception e) {
			Assert.fail("cannot not parse ifconfig");
		} finally {
			if (br != null)
				try {
					br.close();
				} catch (IOException e) {
				}
		}
	}

	@Test
	public void testCentos7LoopbackNetworkInterfaceParse() {
		BufferedReader br = null;
		try {
			br = new BufferedReader(new FileReader(new File("src/test/resources/ifconfig3")));
			NetworkInterface nic = NetworkInterface.parse(br);

			assertEquals("lo", nic.getName());
			assertEquals("local loopback", nic.getLinkEncap());
			assertEquals("", nic.getHwaddr());
			assertEquals("127.0.0.1", nic.getInet().getHostAddress());
			assertEquals(null, nic.getPtp());
			assertEquals("255.0.0.0", nic.getMask().getHostAddress());
			assertEquals("0:0:0:0:0:0:0:1", nic.getInet6().getHostAddress());
			assertEquals(128, nic.getCidr());
			assertEquals("host", nic.getScope());
			assertEquals(65536, nic.getMtu());
			assertEquals(0, nic.getMetric());
			assertEquals(437660, nic.getRxBytes());
			assertEquals(437660, nic.getTxBytes());
			RxPacket rxPacket = nic.getRxPacket();
			assertEquals(7881, rxPacket.getPackets());
			assertEquals(1, rxPacket.getErrors());
			assertEquals(2, rxPacket.getDropped());
			assertEquals(3, rxPacket.getOverruns());
			assertEquals(4, rxPacket.getFrame());
			TxPacket txPacket = nic.getTxPacket();
			assertEquals(7881, txPacket.getPackets());
			assertEquals(5, txPacket.getErrors());
			assertEquals(6, txPacket.getDropped());
			assertEquals(7, txPacket.getOverruns());
			assertEquals(8, txPacket.getCarrier());
			assertEquals(9, txPacket.getCollisions());
			assertEquals(0, txPacket.getQueuelen());
			assertEquals(0, nic.getInterrupt());
			assertEquals(null, nic.getBaseAddress());
			assertEquals(null, nic.getMemory());
		} catch (Exception e) {
			Assert.fail("cannot not parse ifconfig");
		} finally {
			if (br != null)
				try {
					br.close();
				} catch (IOException e) {
				}
		}
	}
}
