#include "module.hh"

extern "C"
{
  void foo();
  extern int bar;
}

std::array<const char*, 2> g_exports = { "foo", "bar" };

class TestModule : public ReloadModule<TestModule, g_exports.size()>
{
public:
  static void Foo() {
    GetInstance().Execute<void>("foo");
  }

  static int GetBar() {
    return *GetInstance().GetVar<decltype(bar)>("bar");
  }

protected:
  virtual const char* GetPath() const override
  {
    return "build/test.o";
  }

  virtual std::array<const char*, g_exports.size()>& GetSymbols() const override
  {
    return g_exports;
  }
};
