project.name = "Putki Test Project"


package = newpackage();
package.path = "build";
package.bindir = ".";
package.name = "test-main"
package.language = "c++"
package.includepaths = { "../../src/cpp-runtime", "../_gen" };
package.files = {
	matchrecursive("../src/*.h", "../src/*.cpp"),
	matchrecursive("../_gen/*.h", "../_gen/*.cpp"),
	matchrecursive("../../src/cpp-runtime/*.cpp", "../../src/cpp-runtime/*.h")
};


