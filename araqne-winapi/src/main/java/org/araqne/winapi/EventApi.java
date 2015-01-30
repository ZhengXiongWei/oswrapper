package org.araqne.winapi;

import java.io.IOException;
import java.util.Iterator;
import java.util.List;

public class EventApi {
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

	public EventApi(String channelPath, String query, String bookmark, int timeout) {
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

	public static void main(String argv[]) {
		System.out.println("Hello, world");
		String[] sid = EventApi.lookupAccountSid("S-1-5-18");
		if ( sid == null )
			System.out.println( "lookupAccountSid error" );
		else {
			System.out.println( sid[0] );
			System.out.println( sid[1] );
		}
		
		try {
			System.in.read();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		
		List<String> channelPaths = EventApi.getChannelPaths();
		for( Iterator<String> itr = channelPaths.iterator(); itr.hasNext(); )
		{
			String channelPath = itr.next();
			System.out.println(channelPath);
		}
		
		System.out.println("Press any key to continue...");
		try {
			System.in.read();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		String bookmark = null;//"<BookmarkList><Bookmark Channel='System' RecordId='78741' IsCurrent='true'/></BookmarkList>";

		EventApi ea = new EventApi("System", "*", bookmark, 1000);
		for ( ;; )
		{
			String msg[] = ea.read();
			if ( msg == null )
				break;
			for (int i = 0; i < msg.length; i++)
				System.out.println(msg[i]);
		}
		ea.close();
	}
}
