package org.araqne.winapi;

import java.util.List;

public class EventLogApi {
	static {
		System.loadLibrary("winapi");
	}

	private native long subscribe(String channelPath, String query,
			String bookmark);
	private native void close(long handle);
	private native String[] read(long handle, int timeout);
	public native static List<String> getChannelPaths();
	public native static String[] lookupAccountSid(String sid);

	private long handle;
	private int timeout;

	public EventLogApi(String channelPath, String query, String bookmark, int timeout) {
		this.timeout = timeout;
		handle = subscribe(channelPath, query, bookmark);
	}

	public void close() {
		if (handle != 0)
			close(handle);
	}

	public String[] read() {
		if (handle != 0)
			return read(handle, timeout);
		else
			return null;
	}
}
