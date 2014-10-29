package org.araqne.winapi;

import java.util.Iterator;

public class Test {
	public static void main(String[] args) {
		long count = 0;
		int skip = 0;
		int limit = Integer.MAX_VALUE;
		if (args.length > 1)
			skip = Integer.parseInt(args[1]);
		if (args.length > 2)
			limit = Integer.parseInt(args[2]);
		EventLogReader reader = new EventLogReader(args[0]);
		Iterator<EventLog> it = reader.iterator(skip);

		while (it.hasNext()) {
			EventLog el = it.next();
			if (limit-- == 0) {
				break;
			}
			System.out.println(el.getRecordNumber());
			System.out.println(el.getWritten());
			System.out.println(el.getMessage());
			System.out.println("================================================");
			count++;
		}
		System.out.println("count: " + count);
	}
}
