/*
 * Ho Chi Minh City University of Technology
 * Faculty of Computer Science and Engineering
 * Initial code for Assignment 2
 * Programming Fundamentals Spring 2025
 * Date: 02.02.2025
 */

#ifndef _H_HCM_CAMPAIGN_H_
#define _H_HCM_CAMPAIGN_H_

#include "main.h"

////////////////////////////////////////////////////////////////////////
/// STUDENT'S ANSWER BEGINS HERE
/// Complete the following functions
/// DO NOT modify any parameters in the functions.
////////////////////////////////////////////////////////////////////////

// Forward declaration
class Unit;
class UnitList;
class Army;
class TerrainElement;

class Vehicle;
class Infantry;

class LiberationArmy;
class ARVN;

class Position;

class Road;
class Mountain;
class River;
class Urban;
class Fortification;
class SpecialZone;

class BattleField;

class HCMCampaign;
class Configuration;

enum VehicleType {
    TRUCK,
    MORTAR,
    ANTIAIRCRAFT,
    ARMOREDCAR,
    APC,
    ARTILLERY,
    TANK
};

enum InfantryType {
    SNIPER,
    ANTIAIRCRAFTSQUAD,
    MORTARSQUAD,
    ENGINEER,
    SPECIALFORCES,
    REGULARINFANTRY
};

class Position {
private:
    int r, c;

public:
    Position(int r = 0, int c = 0);
    Position(const string &str_pos); // Example: str_pos = "(1,15)"
    int getRow() const;
    int getCol() const;
    void setRow(int r);
    void setCol(int c);
    string str() const; // Example: returns "(1,15)"
    bool operator==(const Position& other) const {
        return r == other.r && c == other.c;
    }
};

class Unit {
protected:
    int quantity, weight;
    Position pos;

public:
    Unit(int quantity, int weight, Position pos);
    virtual ~Unit();
    
    virtual int getAttackScore() = 0;
    Position getCurrentPosition() const;
    virtual string str() const = 0;
    void setQuantity(int q) { quantity = q; }
    void setWeight(int w) { weight = w; }
    int getQuantity() const { return quantity; }
    int getWeight() const { return weight; }

    virtual void multiplyAttackScore(double factor) = 0; // pure virtual, bắt buộc lớp con cài đặt
    virtual void setAttackScoreMultiplier(double factor) = 0;
    virtual void resetAttackScoreModifier() = 0;

    virtual Unit* clone() const = 0;
};

class Vehicle : public Unit {
protected:
    VehicleType vehicleType;
    int baseAttackScore;
    double attackScoreMultiplier = 1.0;
    int attackScoreModifier = 0;
public:
    Vehicle(int quantity, int weight, Position pos, VehicleType vehicleType);
    string str() const override;
    VehicleType getVehicleType() const { return vehicleType; }
    Unit* clone() const override { return new Vehicle(*this); }

    void setAttackScoreMultiplier(double m) override { attackScoreMultiplier = m; }
    double getAttackScoreMultiplier() const { return attackScoreMultiplier; }
    void multiplyAttackScore(double m) override { attackScoreMultiplier *= m; }

    void addAttackScoreModifier(int delta) { attackScoreModifier += delta; }
    int getAttackScoreModifier() const { return attackScoreModifier; }
    void resetAttackScoreModifier() override{ attackScoreModifier = 0; }

    int getRawAttackScore() const;

    int getAttackScore() override {
    int score = static_cast<int>(getRawAttackScore() * attackScoreMultiplier + 0.5);
    return score + attackScoreModifier;
    }
};

class Infantry : public Unit {
protected:
    InfantryType infantryType;
    double attackScoreMultiplier = 1.0;
    int attackScoreModifier = 0; 
public:
    Infantry(int quantity, int weight, Position pos, InfantryType infantryType);
    string str() const override;
    InfantryType getInfantryType() const { return infantryType; }
    Unit* clone() const override {return new Infantry(*this);}

    void setAttackScoreMultiplier(double m) override { attackScoreMultiplier = m; }
    double getAttackScoreMultiplier() const { return attackScoreMultiplier; }
    void multiplyAttackScore(double m) override { attackScoreMultiplier *= m; }

    void addAttackScoreModifier(int delta) { attackScoreModifier += delta; }
    int getAttackScoreModifier() const { return attackScoreModifier; }
    void resetAttackScoreModifier() override { attackScoreModifier = 0; }

    int getRawAttackScore();

    int getAttackScore() override {
    int score = static_cast<int>(getRawAttackScore() * attackScoreMultiplier + 0.5);
    return score + attackScoreModifier;
    }
};

class UnitList {
private:
    vector<Unit *> units;
    struct Node {
        Unit* unit;
        Node* next;
        Node(Unit* u) : unit(u), next(nullptr) {}
    };
    Node* head; // Đầu danh sách
    Node* tail; // Cuối danh sách
    int capacity; // Sức chứa tối đa
    int count_vehicle; // Số lượng Vehicle
    int count_infantry; // Số lượng Infantry
    int size; // Số lượng đơn vị hiện tại

public:
    UnitList(int capacity);
    ~UnitList();
    void* getHead() const { return head; };
    Unit* getUnit(void* node) const { return node ? static_cast<Node*>(node)->unit : nullptr; }
    void* getNext(void* node) const { return node ? static_cast<Node*>(node)->next : nullptr; }

    bool insert(Unit *unit);                   // return true if insert successfully
    bool isContain(VehicleType vehicleType);   // return true if it exists
    bool isContain(InfantryType infantryType); // return true if it exists
    string str() const;
    vector<Unit*> getUnits() const;
    bool removeUnit(Unit* unit);
    void removeUnits();
};

class Army {
protected:
    int LF, EXP;
    string name;
    UnitList *unitList;
    BattleField *battleField;

public:
    Army(const Unit **unitArray, int size, string name, BattleField *battleField);
        virtual ~Army() { delete unitList;}
    virtual void fight(Army *enemy, bool defense = false) = 0;
    virtual string str() const = 0;
    int getLF() const { return LF; }
    void setLF(int lf) { this->LF = lf; }
    int getEXP() const { return EXP; }
    void setEXP(int exp) { this->EXP = exp; }
    UnitList* getUnitList() const { return unitList; }  
};

class LiberationArmy : public Army {
public:
    LiberationArmy(const Unit **unitArray, int size, BattleField *battleField);
    void fight(Army *enemy, bool defense = false) override;
    string str() const override;
    void updateStatsFromUnitList();
    void absorbUnits(const vector<Unit*>& unitsToAdd);
    void lose10PercentWeight();
    void lose10PercentQuantity();
    void reinforceFibo();
};

class ARVN : public Army {
public:
    ARVN(const Unit **unitArray, int size, BattleField *battleField);
    void fight(Army *enemy, bool defense = false) override;
    string str() const override;
    void updateStatsFromUnitList(ARVN* army); 
};

class TerrainElement {
public:
    TerrainElement();
    virtual ~TerrainElement();
    virtual void getEffect(Army *army) = 0;
    static int distance(const Position& p1, const Position& p2);
};

class Road : public TerrainElement {
public:
    Road() {}
    ~Road() override {}
    void getEffect(Army *army) override;
};

class Forest : public TerrainElement {
private:
    Position pos;
public:
    Forest(Position position) : pos(position) {}
    ~Forest() override {}
    void getEffect(Army *army) override;
};

class River : public TerrainElement {
private:
    Position pos;
public:
    River(Position position) : pos(position) {}
    ~River() override {}
    void getEffect(Army *army) override;
};

class Urban : public TerrainElement {
private:
    Position pos;
public:
    Urban(Position position) : pos(position) {}
    ~Urban() override {}
    void getEffect(Army *army) override;
};

class Fortification : public TerrainElement {
private:
    Position pos;
public:
    Fortification(Position position) : pos(position) {}
    ~Fortification() override {}
    void getEffect(Army *army) override;
};

class SpecialZone : public TerrainElement {
private:
    Position pos;
public:
    SpecialZone(Position position) : pos(position) {}
    ~SpecialZone() override {}
    void getEffect(Army *army) override;
};

class BattleField {
private:
    int n_rows, n_cols;
    // có thể dùng map từ Position -> TerrainElement nếu cần
    std::vector<std::vector<TerrainElement*>> terrain;
    LiberationArmy* liberationArmy;
    ARVN* arvn;
    bool contains(const std::vector<Position*>& arr, const Position& pos) const {
    for (auto p : arr)
        if (*p == pos) return true;
    return false;
    }
public:
    BattleField(int n_rows, int n_cols, vector<Position *> arrayForest,
                vector<Position *> arrayRiver, vector<Position *> arrayFortification,
                vector<Position *> arrayUrban, vector<Position *> arraySpecialZone);
    ~BattleField();
    string str() const;
    int getRows() const { return n_rows; }
    int getCols() const { return n_cols; }
    void applyTerrainEffects() const;
    void applyTerrainEffects(int row, int col, Army* army) const;
    void battle();
};

class Configuration {
private:
    int num_rows;
    int num_cols;
    std::vector<Position*> arrayForest;
    std::vector<Position*> arrayRiver;
    std::vector<Position*> arrayFortification;
    std::vector<Position*> arrayUrban;
    std::vector<Position*> arraySpecialZone;
    std::vector<Unit*> liberationUnits;
    std::vector<Unit*> ARVNUnits;
    int eventCode;

    static string trim(const std::string& s);
    static Position* parsePosition(const std::string& s);
    void parsePositionArray(const std::string& line, std::vector<Position*>& arr);
    void parseUnitArray(const std::string& line, std::vector<Unit*>& liber, std::vector<Unit*>& arvn);
    int parseInt(const std::string& line);
public:
    Configuration(const std::string& filepath);
    ~Configuration();
    string str() const;
    int getNumRows() const { return num_rows; }
    int getNumCols() const { return num_cols; }
    const vector<Position*>& getArrayForest() const { return arrayForest; } // Thêm getter
    const vector<Position*>& getArrayRiver() const { return arrayRiver; } // Thêm getter
    const vector<Position*>& getArrayFortification() const { return arrayFortification; } // Thêm getter
    const vector<Position*>& getArrayUrban() const { return arrayUrban; } // Thêm getter
    const vector<Position*>& getArraySpecialZone() const { return arraySpecialZone; } // Thêm getter
    const std::vector<Unit*>& getLiberationUnits() const { return liberationUnits; }
    const std::vector<Unit*>& getARVNUnits() const { return ARVNUnits; }
};

class HCMCampaign {
private:
    Configuration *config;
    BattleField *battleField;
    LiberationArmy *liberationArmy;
    ARVN *arvn;

public:
    HCMCampaign(const string &config_file_path);
    void run();
    std::string printResult() const;
};

////////////////////////////////////////////////////////////////////////
/// STUDENT'S ANSWER ENDS HERE
////////////////////////////////////////////////////////////////////////

#endif
