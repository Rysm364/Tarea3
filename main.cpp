#include <iostream>
#include <map>
#include <list>
#include <string>
#include <functional>
using namespace std;

class Entity {
private:
    int x, y;
    int health;
public:
    Entity() : x(0), y(0), health(100) {}

    void move(int dx, int dy) {
        x += dx;
        y += dy;
    }

    void heal(int amount) {
        health += amount;
    }

    void damage(int amount) {
        health -= amount;
    }

    string getState() const {
        return "Pos(" + to_string(x) + "," + to_string(y) +
               ") HP=" + to_string(health);
    }

    void status() const {
        cout << getState() << endl;
    }
};

using Command = function<void(const list<string>&)>;

class DamageCommand {
private:
    Entity& entity;
    int count;

public:
    DamageCommand(Entity& e) : entity(e), count(0) {}

    void operator()(const list<string>& args) {
        if (args.size() != 1) {
            cout << "Error: damage necesita 1 argumento\n";
            return;
        }
        try {
            int val = stoi(*args.begin());
            entity.damage(val);
            count++;
            cout << "[Damage ejecutado " << count << " veces]\n";
        } catch (...) {
            cout << "Error: argumento invalido\n";
        }
    }
};

class CommandCenter {
private:
    Entity& entity;
    map<string, Command> commands;
    list<string> history;
    map<string, list<pair<string, list<string>>>> macros;

public:
    CommandCenter(Entity& e) : entity(e) {}

    void registerCommand(const string& name, Command cmd) {
        commands[name] = cmd;
    }

    void execute(const string& name, const list<string>& args) {
        auto it = commands.find(name);
        if (it == commands.end()) {
            cout << "Error: comando no existe -> " << name << "\n";
            return;
        }
        string before = entity.getState();
        it->second(args);
        string after = entity.getState();
        history.push_back(name + " | " + before + " -> " + after);
    }

    void removeCommand(const string& name) {
        auto it = commands.find(name);
        if (it != commands.end()) {
            commands.erase(it);
            cout << "Comando eliminado: " << name << "\n";
        } else {
            cout << "No existe comando: " << name << "\n";
        }
    }

    void showHistory() {
        cout << "\n--- HISTORIAL ---\n";
        for (auto it = history.begin(); it != history.end(); ++it) {
            cout << *it << "\n";
        }
    }

    void registerMacro(const string& name, const list<pair<string, list<string>>>& steps) {
        macros[name] = steps;
    }

    void executeMacro(const string& name) {
        auto it = macros.find(name);
        if (it == macros.end()) {
            cout << "Macro no existe: " << name << "\n";
            return;
        }
        for (auto step = it->second.begin(); step != it->second.end(); ++step) {
            string cmdName = step->first;
            list<string> args = step->second;
            if (commands.find(cmdName) == commands.end()) {
                cout << "Error en macro, comando no existe: " << cmdName << "\n";
                return;
            }
            execute(cmdName, args);
        }
    }
};

void healCommand(Entity& entity, const list<string>& args) {
    if (args.size() != 1) {
        cout << "Error: heal necesita 1 argumento\n";
        return;
    }
    try {
        int val = stoi(*args.begin());
        entity.heal(val);
    } catch (...) {
        cout << "Error: argumento invalido\n";
    }
}

int main() {
    Entity entity;
    CommandCenter center(entity);

    center.registerCommand("move", [&](const list<string>& args) {
        if (args.size() != 2) {
            cout << "Error: move necesita 2 argumentos\n";
            return;
        }
        try {
            auto it = args.begin();
            int x = stoi(*it++);
            int y = stoi(*it);
            entity.move(x, y);
        } catch (...) {
            cout << "Error: argumentos invalidos\n";
        }
    });

    center.registerCommand("heal", [&](const list<string>& args) {
        healCommand(entity, args);
    });

    center.registerCommand("damage", DamageCommand(entity));

    center.registerCommand("status", [&](const list<string>&) {
        entity.status();
    });

    center.registerCommand("reset", [&](const list<string>&) {
        entity = Entity();
    });

    center.execute("status", {});
    center.execute("move", {"5", "3"});
    center.execute("heal", {"10"});
    center.execute("damage", {"7"});
    center.execute("status", {});

    center.registerMacro("combo1", {
        {"heal", {"5"}},
        {"move", {"1", "1"}},
        {"status", {}}
    });

    center.registerMacro("combo2", {
        {"damage", {"10"}},
        {"damage", {"5"}},
        {"status", {}}
    });

    center.registerMacro("combo3", {
        {"move", {"-2", "4"}},
        {"heal", {"20"}},
        {"status", {}}
    });

    center.executeMacro("combo1");
    center.executeMacro("combo2");
    center.executeMacro("combo3");

    center.showHistory();

    center.removeCommand("heal");
    center.execute("heal", {"10"});

    cout << "----Comandos Invalidos----" << endl;
    center.execute("heal", {});
    center.execute("heal", {"a"});
    center.execute("move", {"1"});
    center.execute("move", {"x", "y"});
    center.execute("fake", {});
    cout << "------------------------" << endl;

    return 0;
}