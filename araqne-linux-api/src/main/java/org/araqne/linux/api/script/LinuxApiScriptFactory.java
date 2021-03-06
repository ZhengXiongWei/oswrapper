/*
 * Copyright 2011 Future Systems
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
package org.araqne.linux.api.script;

import org.apache.felix.ipojo.annotations.Component;
import org.apache.felix.ipojo.annotations.Provides;
import org.apache.felix.ipojo.annotations.ServiceProperty;
import org.araqne.api.Script;
import org.araqne.api.ScriptFactory;

@Component(name = "linux-api-script-factory")
@Provides
public class LinuxApiScriptFactory implements ScriptFactory {
	@SuppressWarnings("unused")
	@ServiceProperty(name = "alias", value = "linux")
	private String alias;

	@Override
	public Script createScript() {
		return new LinuxApiScript();
	}

}
