from subprocess import call
import xml.etree.ElementTree as ET

#call("msinfo32 /nfo sys.nfo")

tree = ET.parse('sys.nfo')
root = tree.getroot()


for data in root.findall(".//Data"):
	if data.find("Item") is not None:
		if (data.find("Item").text == "OS Name"):
			osname = data.find("Value").text
		if (data.find("Item").text == "System Manufacturer"):
			sysman = data.find("Value").text
		if (data.find("Item").text == "System Model"):
			sysmod = data.find("Value").text
		if (data.find("Item").text == "Processor"):
			proc = data.find("Value").text
		if (data.find("Item").text == "Installed Physical Memory (RAM)"):
			ram = data.find("Value").text
			
drives = []

for category in root.findall(".//Category"):
	if (category.attrib['name'] == "Disks"):
		for data in category.findall("Data"):
			if data.find("Item") is not None:
				if (data.find("Item").text == "Size"):
					drives.append(data.find("Value").text)


print(osname)
print(sysman)
print(sysmod)
print(proc)
print(ram)			
print(drives)
