package org.araqne.winapi;

import java.util.Iterator;

public class Test {
	public static void main(String[] args) {
		long count = 0;
		while (true) {
			EventLogReader reader = new EventLogReader(args[0]);
			Iterator<EventLog> it = reader.iterator();

			while (it.hasNext()) {
				EventLog el = it.next();
				System.out.println(el.getMessage());

				if (++count % 1000 == 0) {
					return;
				}
			}
		}
	}
}
