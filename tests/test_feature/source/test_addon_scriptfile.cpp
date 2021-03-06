#include "utils.h"
#include "../../../add_on/scriptfile/scriptfile.h"
#include "../../../add_on/scriptfile/scriptfilesystem.h"

namespace Test_Addon_ScriptFile
{

bool Test()
{
	RET_ON_MAX_PORT

	bool fail = false;
	COutStream out;
	CBufferedOutStream bout;
	int r;
	asIScriptEngine *engine = 0;

	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
		RegisterStdString(engine);
		RegisterScriptArray(engine, false);
		RegisterScriptFile(engine);
		RegisterScriptFileSystem(engine);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test", 
			"void main() { \n"
			"  filesystem fs; \n"										// starts in applications working dir
			"  array<string> dirs = fs.getDirs(); \n"
			"  assert( dirs.find('scripts') >= 0 ); \n"
			"  fs.changeCurrentPath('scripts'); \n"						// move to the sub directory
			"  array<string> files = fs.getFiles(); \n"					// get the script files in the directory
			"  assert( files.length() == 2 ); \n"
			"  file f; \n"
			"  f.open('scripts/include.as', 'r'); \n"
			"  string str = f.readLine(); \n"
			"  str = str.substr(3, 25); \n"
			"  assert( str == 'void MyIncludedFunction()' ); \n"
			"} \n");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		r = ExecuteString(engine, "main()", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		if( bout.buffer != "" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	{
		asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);
		RegisterStdString(engine);
		RegisterScriptFile(engine);

		const char *script =
			"file f;                                                  \n"
			"int r = f.open(\"scripts/TestExecuteScript.as\", \"r\"); \n"
			"if( r >= 0 ) {                                           \n"
			"  assert( f.getSize() > 0 );                             \n"
			"  string s1 = f.readString(10000);                       \n"
			"  assert( s1.length() == uint(f.getSize()) );            \n"
			"  f.close();                                             \n"
			"  f.open('scripts/TestExecuteScript.as', 'r');           \n"
			"  string s2;                                             \n"
			"  while( !f.isEndOfFile() )                              \n"
			"  {                                                      \n"
			"    string s3 = f.readLine();                            \n"
			"    s2 += s3;                                            \n"
			"  }                                                      \n"
			"  assert( s1 == s2 );                                    \n"
			"  f.close();                                             \n"
			"}                                                        \n";

		r = ExecuteString(engine, script);
		if( r != asEXECUTION_FINISHED )
		{
			TEST_FAILED;
		}

		engine->Release();
	}

	return fail;
}

}

