[![Build Status](https://travis-ci.com/russellfrancis/jfuzzy.svg?branch=master)](https://travis-ci.com/russellfrancis/jfuzzy)
# jfuzzy
JNI bindings for the libfuzzy shared library, allowing efficient computation and comparison of 
ssdeep hashes from within Java applications.

## Compilation
In order to compile this library, you will need the following tools installed on your system.

* [JDK 8](https://openjdk.java.net/projects/jdk8u/) or greater. 
* [Apache Maven](https://maven.apache.org/).
* [libfuzzy](https://github.com/ssdeep-project/ssdeep)
* [gcc](https://gcc.gnu.org/)

### Install prerequisites on Debian

`sudo apt-get install libfuzzy-dev build-essential openjdk-8-jdk maven`

### Compile JFuzzy

`mvn install site`

## Usage

After a successful build, there will be two artifacts which are generated and necessary to use the
JFuzzy JNI bindings.  The first is *<root>/jfuzzy/target/jfuzzy-<version>.jar*, this contains the 
Java bindings and must be on the classpath of the Java application consuming the library.  The
second is *<root>/jfuzzy-native/linux/target/libfuzzy.so* which is the native portion of the bindings.
This shared library must be installed on the LD_LIBRARY_PATH used by Java.

An easy way to ensure that it is on the path is to pass the following system property to the JVM 
when it starts

`java -Djava.library.path=/my/path/to/shared/lib -jar app.jar`

Alternatively dropping it into */usr/local/lib* or */usr/lib* will also work in most cases.
