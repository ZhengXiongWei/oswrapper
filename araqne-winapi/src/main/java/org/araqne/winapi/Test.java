package org.araqne.winapi;

import java.util.Iterator;

public class Test {
	public static void main(String[] args) {
		long count = 0;
		while (true) {
			EventLogReader reader = new EventLogReader("Security");
			Iterator<EventLog> it = reader.iterator();

			while (it.hasNext()) {
				it.next();

				if (count++ % 1000 == 0)
					System.out.println(count);
			}

		}
	}
}
