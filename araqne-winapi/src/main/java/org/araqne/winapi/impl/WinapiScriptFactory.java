package org.araqne.winapi.impl;

import org.apache.felix.ipojo.annotations.Component;
import org.apache.felix.ipojo.annotations.Provides;
import org.apache.felix.ipojo.annotations.ServiceProperty;
import org.araqne.api.Script;
import org.araqne.api.ScriptFactory;

@Component(name = "winapi-script-factory")
@Provides
public class WinapiScriptFactory implements ScriptFactory {

	@ServiceProperty(name = "alias", value = "winapi")
	private String alias;

	@Override
	public Script createScript() {
		return new WinapiScript();
	}

}
