/*
 * Copyright 2010 NCHOVY
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
package org.araqne.winapi;

public class RoutingTable {
	static {
		System.loadLibrary("winapi");
	}
	
	public enum Type {
		Direct, Indirect, Invalid, Other
	};

	public enum Protocol {
		Other, Local, NetMgmt, ICMP, EGP, GGP, Hello, RIP, IS_IS, ES_IS, IGRP, BBN, OSPF, BGP, Autostatic, Static, StaticNonDOD
	};

	public static native RoutingEntry[] getRoutingEntries();
}
