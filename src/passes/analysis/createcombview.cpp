#include "coreir.h"
#include "coreir/passes/analysis/createcombview.h"

using namespace std;
using namespace CoreIR;

void Passes::CreateCombView::setupCoreir(Module* m) {
  string mname = m->getName();
  if (mname == "reg" || mname == "regRst") {
    srcs[m].insert({"out"});
    snks[m].insert({"in"});
    snks[m].insert({"clk"});
  }
  else if (mname == "mem") {
    for (auto record : m->getType()->getRecord()) {
      if (record.second->isInput()) {
        snks[m].insert({record.first});
      }
      else {
        assert(record.second->isOutput());
        srcs[m].insert({record.first});
      }
    }
  }
  else {
    set<SelectPath> inputs;
    set<SelectPath> outputs;
    for (auto record : m->getType()->getRecord()) {
      if (record.second->isInput()) {
        inputs.insert({record.first});
      }
      else {
        assert(record.second->isOutput());
        outputs.insert({record.first});
      }
    }
    combs[m].inputs = inputs;
    combs[m].outputs = outputs;
  }
}

struct Output {
  set<Wireable*> states;
  set<Wireable*> inputs;
};

struct Input {
  set<Wireable*> states;
  set<Wireable*> outputs; //Unused for now
};


string Passes::CreateCombView::ID = "createcombview";
bool Passes::CreateCombView::runOnInstanceGraphNode(InstanceGraphNode& node) {
  Module* m = node.getModule();
  if (m->getNamespace()->getName() == "coreir") {
    //Set srcs/snks/comb for coreir
    setupCoreir(m);
    return false;
  }
  if (m->getNamespace()->getName() == "corebit") {
    ASSERT(0,"NYI");
  }
  ASSERT(m->hasDef(), "NEEDS Def!");
  //ModuleDef* mdef = m->getDef();

  DirectedModule dm(m);

  map<Wireable*,Output*> outputInfo;
  map<Wireable*,Input*> inputInfo;
  
  for (auto outcon : dm.getOutputs()) {
    Wireable* output = outcon->getSnkWireable();
    assert(output->getType()->isInput()); //because reversed
    outputInfo.emplace(output,new Output());
  }
  
  for (auto incon : dm.getInputs()) {
    Wireable* input = incon->getSrcWireable();
    assert(input->getType()->isOutput());
    inputInfo.emplace(input,new Input());
  }
  
  //Find all combinational dependencies
  //for (auto outcon : dm.getOutputs()) {
  //  Wireable* output = outcon->getSnkWireable();
  //  traverseOut2In(output,output);
  //}
  //
  //for (auto outcon : dm.getOutputs()) {
  //  Wireable* output = outcon->getSnkWireable();
  //  if (outputInfo.inputs.count(output)==0) {
  //    outputInfo.states.insert(output); //Not sure why I am adding this here
  //  }
  //}

  //for (auto ipair : mdef->getInstances()) {
  //  Module* mref = ipair->getModuleRef();
  //  if (!this->hasSrc(mref)) continue;
  //  for (auto 
  //}


  for (auto opair : outputInfo) {
    Output* oinfo = opair.second;
    Wireable* out = opair.first;
    SelectPath opath = out->getSelectPath();
    assert(opath[0]=="self");
    opath.pop_front();
    if (oinfo->states.size()>0) {
      srcs[m].insert(opath);
    }
    for (auto in : oinfo->inputs) {
      SelectPath ipath = in->getSelectPath();
      assert(ipath[0]=="self");
      ipath.pop_front();
      combs[m].inputs.insert(ipath);
      combs[m].outputs.insert(opath);
    }
  }

  for (auto ipair : inputInfo) {
    Input* iinfo = ipair.second;
    Wireable* in = ipair.first;
    if (iinfo->states.size()>0) {
      SelectPath ipath = in->getSelectPath();
      assert(ipath[0]=="self");
      ipath.pop_front();
      snks[m].insert(ipath);
    }
  }
  
  //cleanup
  for (auto opair : outputInfo) delete opair.second;
  for (auto ipair : inputInfo) delete ipair.second;
  
  return false;
}

//void traverseOut2In(Wireable* curout, Wireable* out) {
//  Wireable* input = foo(curout);
//  Wireable* parent = input->getTopParent();
//  if (isa<Interface>(parent)) {
//    outputInfo.inputs.insert(input);
//    inputInfo.outputs.insert(out);
//    return;
//  }
//  Instance* inode = cast<Instance>(parent);
//  Module* mnode = inode->getModuleRef();
//  if (this->hasComb(mnode)) {
//    auto checkoutputs = combs[mnode].outputs;
//    //TODO check that input is in checkoutputs
//    for (auto nextin : combs[mnode].inputs) {
//      traverseOut2In(nextin, out);
//    }
//  }

//}
