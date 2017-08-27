#include "coreir-lib/cgralib.h"

COREIR_GEN_C_API_DEFINITION_FOR_LIBRARY(cgralib);

using namespace CoreIR;

Namespace* CoreIRLoadLibrary_cgralib(Context* c) {
  
  Namespace* cgralib = c->newNamespace("cgralib");
  
  //Unary op declaration
  Params widthParams = {{"width",AINT}};
  cgralib->newTypeGen("unary",widthParams,[](Context* c, Args args) { 
    uint width = args.at("width")->get<ArgInt>();
    return c->Record({
      {"in",c->BitIn()->Arr(width)},
      {"out",c->Bit()->Arr(width)},
    });
  });

  //PE declaration
  Params PEGenParams = {{"width",AINT},{"numbitports",AINT},{"numdataports",AINT}};
  Params PEConfigParams({
    {"op",ASTRING},
    {"LUT_init",AINT},
    {"data0_mode",ASTRING},
    {"data1_mode",ASTRING},
    {"bit0_mode",ASTRING}
  });
  cgralib->newTypeGen("PEType",PEGenParams,[](Context* c, Args args) {
    uint width = args.at("width")->get<ArgInt>();
    uint numdataports = args.at("numdataports")->get<ArgInt>();
    uint numbitports = args.at("numbitports")->get<ArgInt>();
    return c->Record({
      {"data",c->Record({
        {"in",c->BitIn()->Arr(width)->Arr(numdataports)},
        {"out",c->Bit()->Arr(width)}
      })},
      {"bit",c->Record({
        {"in",c->BitIn()->Arr(numbitports)},
        {"out",c->Bit()}
      })}
    });
  });
  Generator* PE = cgralib->newGeneratorDecl("PE",cgralib->getTypeGen("PEType"),PEGenParams,PEConfigParams);
  PE->setDefaultGenArgs({{"width",c->argInt(16)},{"numdataports",c->argInt(2)},{"numbitports",c->argInt(3)}});
  PE->setDefaultConfigArgs({
      {"LUT_init",c->argInt(0)},
      {"data0_mode",c->argString("BYPASS")},
      {"data1_mode",c->argString("BYPASS")},
      {"bit0_mode",c->argString("BYPASS")}
  });

  //DataPE declaration
  Params DataPEGenParams = {{"width",AINT},{"numdataports",AINT}};
  Params DataPEConfigParams({
    {"op",ASTRING},
    {"data0_mode",ASTRING},
    {"data1_mode",ASTRING}
  });

  cgralib->newTypeGen("DataPEType",DataPEGenParams,[](Context* c, Args args) {
    uint width = args.at("width")->get<ArgInt>();
    uint numdataports = args.at("numdataports")->get<ArgInt>();
    return c->Record({
      {"data",c->Record({
        {"in",c->BitIn()->Arr(width)->Arr(numdataports)},
        {"out",c->Bit()->Arr(width)}
      })}
    });
  });
  Generator* DataPE = cgralib->newGeneratorDecl("DataPE",cgralib->getTypeGen("DataPEType"),DataPEGenParams,DataPEConfigParams);
  DataPE->setDefaultGenArgs({{"width",c->argInt(16)},{"numdataports",c->argInt(2)}});
  DataPE->setDefaultConfigArgs({
      {"data0_mode",c->argString("BYPASS")},
      {"data1_mode",c->argString("BYPASS")}
  });
  
  //BitPE declaration
  Params BitPEGenParams = {{"numbitports",AINT}};
  Params BitPEConfigParams({
    {"LUT_init",AINT},
    {"bit0_mode",ASTRING},
    {"bit1_mode",ASTRING}
  });

  cgralib->newTypeGen("BitPEType",BitPEGenParams,[](Context* c, Args args) {
    uint numbitports = args.at("numbitports")->get<ArgInt>();
    return c->Record({
      {"bit",c->Record({
        {"in",c->BitIn()->Arr(numbitports)},
        {"out",c->Bit()}
      })}
    });
  });
  Generator* BitPE = cgralib->newGeneratorDecl("BitPE",cgralib->getTypeGen("BitPEType"),BitPEGenParams,BitPEConfigParams);
  BitPE->setDefaultGenArgs({{"numbitports",c->argInt(2)}});
  BitPE->setDefaultConfigArgs({
    {"bit0_mode",c->argString("BYPASS")}
  });

  //IO Declaration
  Params modeParams = {{"mode",ASTRING}};
  cgralib->newGeneratorDecl("IO",cgralib->getTypeGen("unary"),widthParams,modeParams);

  //Mem declaration
  Params MemGenParams = {{"width",AINT},{"depth",AINT}};
  cgralib->newTypeGen("MemType",MemGenParams,[](Context* c, Args args) {
    uint width = args.at("width")->get<ArgInt>();
    return c->Record({
      {"addr", c->BitIn()->Arr(width)},
      {"rdata", c->Bit()->Arr(width)},
      {"ren", c->BitIn()},
      {"empty", c->Bit()},
      {"wdata", c->BitIn()->Arr(width)},
      {"wen", c->BitIn()},
      {"full", c->Bit()}
    });
  });
  cgralib->newGeneratorDecl("Mem",cgralib->getTypeGen("MemType"),MemGenParams,modeParams);

  //Declare a TypeGenerator (in global) for linebuffer
  cgralib->newTypeGen(
    "linebuffer_type", //name for the typegen
    {{"stencil_width",AINT},{"stencil_height",AINT},{"image_width",AINT},{"bitwidth",AINT}}, //generater parameters
    [](Context* c, Args args) { //Function to compute type
      uint stencil_width  = args.at("stencil_width")->get<ArgInt>();
      uint stencil_height  = args.at("stencil_height")->get<ArgInt>();
      //uint image_width = args.at("image_width")->get<ArgInt>();
      uint bitwidth = args.at("bitwidth")->get<ArgInt>();
      return c->Record({
	  {"in",c->BitIn()->Arr(bitwidth)},
	  {"out",c->Bit()->Arr(bitwidth)->Arr(stencil_width)->Arr(stencil_height)}
      });
    }
  );


  Generator* linebuffer = cgralib->newGeneratorDecl("Linebuffer",
					      cgralib->getTypeGen("linebuffer_type"),
					      {{"stencil_width",AINT},{"stencil_height",AINT},
					       {"image_width",AINT},{"bitwidth",AINT}});
  linebuffer->setGeneratorDefFromFun([](ModuleDef* def,Context* c, Type* t, Args args) {
    uint stencil_width  = args.at("stencil_width")->get<ArgInt>();
    uint stencil_height  = args.at("stencil_height")->get<ArgInt>();
    uint image_width = args.at("image_width")->get<ArgInt>();
    uint bitwidth = args.at("bitwidth")->get<ArgInt>();
    assert((bitwidth & (bitwidth-1)) == 0); //Check if power of 2
    assert(stencil_height > 0);
    assert(stencil_width > 0);
    assert(image_width > stencil_width);
    assert(bitwidth > 0);

    Namespace* cgralib = CoreIRLoadLibrary_cgralib(c);
    Generator* Mem = cgralib->getGenerator("Mem");
    Generator* Reg = cgralib->getGenerator("Reg");
    Arg* aBitwidth = c->argInt(bitwidth);
    Arg* aImageWidth = c->argInt(image_width);

    // create the inital register chain
    std::string reg_prefix = "reg_";
    for (uint j = 1; j < stencil_width; ++j) {

      std::string reg_name = reg_prefix + "0_" + std::to_string(j);
      def->addInstance(reg_name, Reg, {{"width",aBitwidth}});
      
      // connect the input
      if (j == 1) {
	def->connect({"self","in"}, {reg_name, "in"});
      } else {
	std::string prev_reg = reg_prefix + "0_" + std::to_string(j-1);
	def->connect({prev_reg, "out"}, {reg_name, "in"});
      }
    }

    // connect together the memory lines
    std::string mem_prefix = "mem_";
    for (uint i = 1; i < stencil_height; ++i) {
      std::string mem_name = mem_prefix + std::to_string(i);
      def->addInstance(mem_name,Mem,{{"width",aBitwidth},{"depth",aImageWidth}},{{"mode",c->argString("linebuffer")}});

      // connect the input
      if (i == 1) {
	def->connect({"self","in"}, {mem_name, "wdata"});
      } else {
	std::string prev_mem = mem_prefix + std::to_string(i-1);
	def->connect({prev_mem, "rdata"}, {mem_name, "wdata"});
      }
    }

    // connect together the remaining stencil registers
    for (uint i = 1; i < stencil_height; ++i) {
      for (uint j = 1; j < stencil_width; ++j) {
	std::string reg_name = reg_prefix + std::to_string(i) + "_" + std::to_string(j);
	def->addInstance(reg_name, Reg, {{"width",aBitwidth}});
	
	// connect the input
	if (j == 1) {
	  std::string mem_name = mem_prefix + std::to_string(i);
	  def->connect({mem_name, "rdata"}, {reg_name, "in"});
	} else {
	  std::string prev_reg = reg_prefix + std::to_string(i) + "_" + std::to_string(j-1);
	  def->connect({prev_reg, "out"}, {reg_name, "in"});
	}
      }
    }

    // connect the stencil outputs
    for (uint i = 0; i < stencil_height; ++i) {
      for (uint j = 0; j < stencil_width; ++j) {
        // delays correspond to earlier pixels
        uint iflip = (stencil_height - 1) - i;
        uint jflip = (stencil_width - 1) - j;

        if (j == 0) {
          // the first column comes from input/mem
          if (i == 0) {
            def->connect({"self","in"}, {"self","out",std::to_string(iflip),std::to_string(jflip)});
          } else {
            std::string mem_name = mem_prefix + std::to_string(i);
            def->connect({mem_name, "rdata"}, {"self","out",std::to_string(iflip),std::to_string(jflip)});
          }
        } else {
          // rest come from registers
          std::string reg_name = reg_prefix + std::to_string(i) + "_" + std::to_string(j);
          def->connect({reg_name, "out"}, {"self","out",std::to_string(iflip),std::to_string(jflip)});
        }
      }
    }    

  });
  

  return cgralib;
}


COREIR_GEN_EXTERNAL_API_FOR_LIBRARY(cgralib)
