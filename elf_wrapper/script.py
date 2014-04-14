#!/usr/bin/python
import shutil
import os
import xml.etree.ElementTree as ET

CA_LIB = "libca-activity.so"
CA_SO = "ca-activity"
APP_NAME = "NativeActivity"
FILE = 'AndroidManifest.xml'
REPLACE = "            <meta-data android:name=\"android.app.lib_name\" android:value=\"native-activity\" />"
NEWREPLACE = "            <meta-data android:name=\"android.app.lib_name\" android:value=\"" + CA_SO+"\" />\n"
NEWREPLACE2 = "            <meta-data android:name=\"android.app.func_name\" android:value=\"ANativeActivity\" />\n"
APK = "apktool/NativeActivity.apk"

#objdump = "open /Users/catechnologies/Documents/android-ndk-r9/toolchains/arm-linux-androideabi-4.8/prebuilt/darwin-x86_64/bin"
#native_activity = "open /Users/catechnologies/Documents/android-ndk-r9/samples/native-activity/jni"
#platform_tools = " /Users/catechnologies/Downloads/adt-bundle-mac-x86_64-20130917/sdk/platform-tools"

#os.system(objdump)
#os.system(native_activity)
#os.system(platform_tools)

def LOGE(msg):
	print '\033[91m' + msg + '\033[0m'

def code_Inject():
	shutil.move( APP_NAME+'/'+FILE, APP_NAME+'/'+FILE+"~" )
	destination= open( APP_NAME+'/'+FILE, "w" )
	source= open( APP_NAME+'/'+FILE+"~", "r" )
	for line in source:
		if REPLACE in line:
			destination.write(NEWREPLACE)
			destination.write(NEWREPLACE2)
		else:
			destination.write(line)
	source.close()
	destination.close()
	print "Modified successfully\n"

def Inject():
	try:
		ET.register_namespace('android', 'http://schemas.android.com/apk/res/android')
		tree = ET.parse(APP_NAME+'/AndroidManifest.xml')
		root = tree.getroot()
		for neighbor in root.iter('meta-data'):
			print 'Replaced lib =>'+ neighbor.get('{http://schemas.android.com/apk/res/android}value')
			neighbor.set('{http://schemas.android.com/apk/res/android}value', CA_LIB)
			print 'With lib =>' + neighbor.get('{http://schemas.android.com/apk/res/android}value')
		tree.write(APP_NAME+'/AndroidManifest.xml', encoding='utf-8', xml_declaration=True)
		print "Modified successfully\n"
	except e:
		LOGE("error Injecting\n")


if os.path.isfile(APK) :
	try:
		os.chdir("apktool")
		shutil.copy(APK, ".")
		print "APK copied successfully.."
		os.system("./apktool d -f "+APP_NAME+".apk")
		print "\nAPK disassembled.."
		shutil.copy(CA_LIB, "NativeActivity/lib/armeabi")
		print "\'"+ CA_LIB +"\'"+ " copied successfully.."
		print "Modifying AndroidManifest.xml.."
		code_Inject()
		os.system("./apktool b "+ APP_NAME + " new_" + APP_NAME+".apk")
		print '\033[92m' + "\nWrapping completed successfully\n" + '\033[0m'
	except IOError, e:
		LOGE("error copying APK")
else:
	LOGE("APK file not found")


