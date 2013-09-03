package org.araqne.winapi.impl;

import org.araqne.api.Script;
import org.araqne.api.ScriptContext;
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
}
