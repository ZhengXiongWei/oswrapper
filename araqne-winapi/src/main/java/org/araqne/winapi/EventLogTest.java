package org.araqne.winapi;

import java.util.Iterator;

public class EventLogTest {
	public static void main(String[] args) {
		System.loadLibrary("winapi");

		// EventLogReader reader = new EventLogReader("VisualSVN Server");
		EventLogReader reader = new EventLogReader("Application");
		Iterator<EventLog> it = reader.iterator(25389);

		while (it.hasNext()) {
			EventLog log = it.next();
			System.out.println(log.getRecordNumber());
		}
	}
}
