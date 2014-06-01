/*
 * Copyright 2010 NCHOVY
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
package org.araqne.winapi;

public class SystemTime {
	private int idlePercent;
	private int kernelPercent;
	private int userPercent;

	static {
		System.loadLibrary("winapi");
	}

	public SystemTime() throws InterruptedException {
		this(100);
	}

	public SystemTime(int interval) throws InterruptedException {
		long[] first = getSystemTimes();
		Thread.sleep(interval);
		long[] second = getSystemTimes();

		// unit: 100 nano seconds
		long idleTime = second[0] - first[0];
		long kernelTime = second[1] - first[1];
		long userTime = second[2] - first[2];

		long systemTime = kernelTime + userTime;

		kernelTime = kernelTime - idleTime;

		if (kernelTime < 0)
			kernelTime = 0;

		kernelPercent = (int) Math.round(((double) kernelTime) / systemTime * 100);
		userPercent = (int) Math.round(((double) userTime) / systemTime * 100);
		idlePercent = 100 - userPercent - kernelPercent;
	}

	private native long[] getSystemTimes();

	public int getUsage() {
		int usage = userPercent + kernelPercent;
		return usage > 100 ? 100 : usage;
	}

	public int getIdlePercent() {
		return idlePercent;
	}

	public int getKernelPercent() {
		return kernelPercent;
	}

	public int getUserPercent() {
		return userPercent;
	}

	@Override
	public String toString() {
		return "idle=" + idlePercent + ", kernel=" + kernelPercent + ", user=" + userPercent;
	}
}
