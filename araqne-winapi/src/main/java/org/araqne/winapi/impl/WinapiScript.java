package org.araqne.winapi.impl;

import java.util.Iterator;

import org.araqne.api.Script;
import org.araqne.api.ScriptArgument;
import org.araqne.api.ScriptContext;
import org.araqne.api.ScriptUsage;
import org.araqne.winapi.EventLog;
import org.araqne.winapi.EventLogReader;
import org.araqne.winapi.RegistryKey;

public class WinapiScript implements Script {
	private ScriptContext context;

	@Override
	public void setScriptContext(ScriptContext context) {
		this.context = context;
	}

	public void eventSources(String[] args) {
		RegistryKey key = null;
		try {
			key = RegistryKey.localMachine().openSubKey("SYSTEM\\CurrentControlSet\\Services\\Eventlog");

			context.println("Event Sources");
			context.println("---------------");
			for (String name : key.getSubKeyNames()) {
				context.println(name);
			}
		} finally {
			if (key != null)
				key.close();
		}

	}

	@ScriptUsage(description = "print recent event logs", arguments =
			@ScriptArgument(name = "source name", type = "string", description = "event source name"))
	public void eventLogs(String[] args) {
		EventLogReader reader = new EventLogReader(args[0]);
		Iterator<EventLog> it = reader.iterator();

		long count = 0;
		while (it.hasNext()) {
			if (count >= 10)
				break;
			context.println(it.next());
			count++;
		}
	}
}
