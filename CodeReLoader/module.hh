#include <dlfcn.h>
#include <iostream>
#include <array>
#include <unordered_map>
#include <string>

template<typename E, size_t NumSymbols>
class ReloadModule {
public:
    static void LoadLibrary() { GetInstance().Load(); }
    static void ReloadLibrary() { GetInstance().Reload(); }

protected:
    static E& GetInstance() {
        static E instance;
        return instance;
    }

    virtual const char* GetPath() const = 0;
    virtual std::array<const char*, NumSymbols>& GetSymbols() const = 0;

    template<typename ret, typename... Args>
    ret Execute(const char* name, Args... args) {
        auto symbol = m_symbols.find(name);
        if (symbol != m_symbols.end()) {
            return reinterpret_cast<ret(*)(Args...)>(symbol->second)(args...);
        }
    }

    template <typename T>
    T* GetVar(const char* name) {
        auto symbol = m_symbols.find(name);
        if (symbol != m_symbols.end()) {
            return static_cast<T*>(symbol->second);
        } else {
            return nullptr;
        }
    }

private:
  void Load()
  {
    m_libHandle = dlopen(GetPath(), RTLD_NOW);
    LoadSymbols();
  }

  void Reload()
  {
    dlclose(m_libHandle);
    m_symbols.clear();
    Load();
  }

  void LoadSymbols()
  {
    for (const char* symbol : GetSymbols())
    {
      m_symbols[symbol] = dlsym(m_libHandle, symbol);
    }
  }

  void* m_libHandle;
  std::unordered_map<std::string, void*> m_symbols;
};
