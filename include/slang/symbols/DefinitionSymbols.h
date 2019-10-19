//------------------------------------------------------------------------------
// DefinitionSymbols.h
// Contains definition-related symbol definitions.
//
// File is under the MIT license; see LICENSE for details.
//------------------------------------------------------------------------------
#pragma once

#include "slang/binding/ConstantValue.h"
#include "slang/symbols/SemanticFacts.h"
#include "slang/symbols/Scope.h"
#include "slang/symbols/Symbol.h"
#include "slang/symbols/TimeScaleSymbolBase.h"

namespace slang {

class ModportSymbol;
class NetType;
class ParameterSymbolBase;
class PortSymbol;

/// Represents a definition (module, interface, or program) that can be instantiated
/// to form a node in the design hierarchy.
class DefinitionSymbol : public Symbol, public Scope, TimeScaleSymbolBase {
public:
    span<const ParameterSymbolBase* const> parameters;
    DefinitionKind definitionKind;
    const NetType& defaultNetType;

    DefinitionSymbol(Compilation& compilation, string_view name, SourceLocation loc,
                     DefinitionKind definitionKind, const NetType& defaultNetType);

    const SymbolMap& getPortMap() const {
        ensureElaborated();
        return *portMap;
    }

    /// Looks for a modport in this definition and issues a diagnostic if not found.
    const ModportSymbol* getModportOrError(string_view modport, const Scope& scope,
                                           SourceRange range) const;

    TimeScale getTimeScale() const { return timeScale; }
    void toJson(json& j) const;

    static DefinitionSymbol& fromSyntax(Compilation& compilation,
                                        const ModuleDeclarationSyntax& syntax, const Scope& scope);
    static bool isKind(SymbolKind kind) { return kind == SymbolKind::Definition; }

private:
    SymbolMap* portMap;
};

struct HierarchicalInstanceSyntax;
struct HierarchyInstantiationSyntax;

/// Base class for module, interface, and program instance symbols.
class InstanceSymbol : public Symbol, public Scope {
public:
    const DefinitionSymbol& definition;
    span<const int32_t> arrayPath;

    const SymbolMap& getPortMap() const {
        ensureElaborated();
        return *portMap;
    }

    void toJson(json& j) const;

    static void fromSyntax(Compilation& compilation, const HierarchyInstantiationSyntax& syntax,
                           LookupLocation location, const Scope& scope,
                           SmallVector<const Symbol*>& results);

    static bool isKind(SymbolKind kind);

protected:
    InstanceSymbol(SymbolKind kind, Compilation& compilation, string_view name, SourceLocation loc,
                   const DefinitionSymbol& definition);

    void populate(const HierarchicalInstanceSyntax* syntax,
                  span<const ParameterSymbolBase* const> parameters);

private:
    SymbolMap* portMap;
};

class ModuleInstanceSymbol : public InstanceSymbol {
public:
    ModuleInstanceSymbol(Compilation& compilation, string_view name, SourceLocation loc,
                         const DefinitionSymbol& definition) :
        InstanceSymbol(SymbolKind::ModuleInstance, compilation, name, loc, definition) {}

    static ModuleInstanceSymbol& instantiate(Compilation& compilation, string_view name,
                                             SourceLocation loc,
                                             const DefinitionSymbol& definition);

    static ModuleInstanceSymbol& instantiate(Compilation& compilation,
                                             const HierarchicalInstanceSyntax& syntax,
                                             const DefinitionSymbol& definition,
                                             span<const ParameterSymbolBase* const> parameters);

    static bool isKind(SymbolKind kind) { return kind == SymbolKind::ModuleInstance; }
};

class InterfaceInstanceSymbol : public InstanceSymbol {
public:
    InterfaceInstanceSymbol(Compilation& compilation, string_view name, SourceLocation loc,
                            const DefinitionSymbol& definition) :
        InstanceSymbol(SymbolKind::InterfaceInstance, compilation, name, loc, definition) {}

    static InterfaceInstanceSymbol& instantiate(Compilation& compilation,
                                                const HierarchicalInstanceSyntax& syntax,
                                                const DefinitionSymbol& definition,
                                                span<const ParameterSymbolBase* const> parameters);

    static bool isKind(SymbolKind kind) { return kind == SymbolKind::InterfaceInstance; }
};

class InstanceArraySymbol : public Symbol, public Scope {
public:
    span<const Symbol* const> elements;
    ConstantRange range;

    InstanceArraySymbol(Compilation& compilation, string_view name, SourceLocation loc,
                        span<const Symbol* const> elements, ConstantRange range) :
        Symbol(SymbolKind::InstanceArray, name, loc),
        Scope(compilation, this), elements(elements), range(range) {}

    void toJson(json& j) const;

    static bool isKind(SymbolKind kind) { return kind == SymbolKind::InstanceArray; }
};

} // namespace slang
