#include "hcmcampaign.h"

////////////////////////////////////////////////////////////////////////
/// STUDENT'S ANSWER BEGINS HERE
////////////////////////////////////////////////////////////////////////
Position::Position(int r, int c) : r(r), c(c) {}

Position::Position(const string &str_pos) {
    // Parse "(r,c)" to get r, c
    int open = str_pos.find('(');
    int comma = str_pos.find(',', open);
    int close = str_pos.find(')', comma);
    r = stoi(str_pos.substr(open + 1, comma - open - 1));
    c = stoi(str_pos.substr(comma + 1, close - comma - 1));
}

int Position::getRow() const { return r; }
int Position::getCol() const { return c; }
void Position::setRow(int r) { this->r = r; }
void Position::setCol(int c) { this->c = c; }
string Position::str() const {
    return "(" + to_string(r) + "," + to_string(c) + ")";
}

Unit::Unit(int quantity, int weight, Position pos)
    : quantity(quantity), weight(weight), pos(pos) {}

Unit::~Unit() {}

Position Unit::getCurrentPosition() const { return pos; }

Vehicle::Vehicle(int quantity, int weight, Position pos, VehicleType vehicleType)
    : Unit(quantity, weight, pos), vehicleType(vehicleType) {}

int Vehicle::getRawAttackScore() const {
    int typeValue = static_cast<int>(vehicleType);
    return (typeValue * 304 + quantity * weight) / 30;
}

string Vehicle::str() const {
    string typeStr;
    switch (vehicleType) {
        case TRUCK: typeStr = "TRUCK"; break;
        case MORTAR: typeStr = "MORTAR"; break;
        case ANTIAIRCRAFT: typeStr = "ANTIAIRCRAFT"; break;
        case ARMOREDCAR: typeStr = "ARMOREDCAR"; break;
        case APC: typeStr = "APC"; break;
        case ARTILLERY: typeStr = "ARTILLERY"; break;
        case TANK: typeStr = "TANK"; break;
        default: typeStr = "UNKNOWN"; break;
    }
    return "Vehicle[vehicleType=" + typeStr + ",quantity=" + to_string(quantity) +
           ",weight=" + to_string(weight) + ",pos=" + pos.str() + "]";
}

Infantry::Infantry(int quantity, int weight, Position pos, InfantryType infantryType)
    : Unit(quantity, weight, pos), infantryType(infantryType) {}

static bool isPerfectSquare(int n) {
    if (n < 0) return false;
    int root = static_cast<int>(sqrt(n));
    return root * root == n;
}

int getDigitalRoot(int num) {
    while (num >= 10) {
        int sum = 0;
        while (num > 0) {
            sum += num % 10;
            num /= 10;
        }
        num = sum;
    }
    return num;
}

int Infantry::getRawAttackScore() {
    int typeValue = static_cast<int>(infantryType);
    int score = typeValue * 56 + quantity * weight;

    if (infantryType == SPECIALFORCES && isPerfectSquare(weight)) {
        score += 75;
    }
    int personal_number = getDigitalRoot(score + 1 + 9 + 7 + 5); // +1975
    if (personal_number > 7) {
        score += (quantity / 5) * weight; // chi viện 20%
    } else if (personal_number < 3) {
        score -= (quantity / 10) * weight; // đào ngũ 10%
    }

    // Tính lại score với quantity mới
    score = typeValue * 56 + quantity * weight;
    if (infantryType == SPECIALFORCES && isPerfectSquare(weight)) {
        score += 75;
    }
    return score;
}

string Infantry::str() const {
    string typeStr;
    switch (infantryType) {
        case SNIPER: typeStr = "SNIPER"; break;
        case ANTIAIRCRAFTSQUAD: typeStr = "ANTIAIRCRAFTSQUAD"; break;
        case MORTARSQUAD: typeStr = "MORTARSQUAD"; break;
        case ENGINEER: typeStr = "ENGINEER"; break;
        case SPECIALFORCES: typeStr = "SPECIALFORCES"; break;
        case REGULARINFANTRY: typeStr = "REGULARINFANTRY"; break;
        default: typeStr = "UNKNOWN"; break;
    }
    return "Infantry[infantryType=" + typeStr + ",quantity=" + to_string(quantity) +
           ",weight=" + to_string(weight) + ",pos=" + pos.str() + "]";
}

Army::Army(const Unit** unitArray, int size, string name, BattleField* battleField)
    : LF(0), EXP(0), name(name), battleField(battleField) {
    unitList = new UnitList(size);
    for (int i = 0; i < size; ++i) {
        if (unitArray[i]) {
            unitList->insert(const_cast<Unit*>(unitArray[i]));
            Vehicle* vehicle = dynamic_cast<Vehicle*>(const_cast<Unit*>(unitArray[i]));
            Infantry* infantry = dynamic_cast<Infantry*>(const_cast<Unit*>(unitArray[i]));
            if (vehicle) {
                LF += vehicle->getAttackScore();
            } else if (infantry) {
                EXP += infantry->getAttackScore();
            }
        }
    }
    if (LF > 1000) LF = 1000;
    if (LF < 0) LF = 0;
    if (EXP > 500) EXP = 500;
    if (EXP < 0) EXP = 0;
}

LiberationArmy::LiberationArmy(const Unit** unitArray, int size, BattleField* battleField)
    : Army(unitArray, size, "LiberationArmy", battleField) {}

// Helper: Fibonacci gần nhất >= n
static int nearestFibo(int n) {
    if (n <= 1) return 1;
    int a = 1, b = 1;
    while (b < n) {
        int t = b;
        b = a + b;
        a = t;
    }
    return b;
}

// Helper: lấy danh sách Infantry*
static vector<Infantry*> getInfantries(UnitList* unitList) {
    vector<Infantry*> res;
    for (auto u : unitList->getUnits())
        if (auto inf = dynamic_cast<Infantry*>(u)) res.push_back(inf);
    return res;
}

// Helper: lấy danh sách Vehicle*
static vector<Vehicle*> getVehicles(UnitList* unitList) {
    vector<Vehicle*> res;
    for (auto u : unitList->getUnits())
        if (auto v = dynamic_cast<Vehicle*>(u)) res.push_back(v);
    return res;
}

// Helper: tìm tổ hợp nhỏ nhất tổng score > target
template<typename T>
static bool findMinSubsetSumGreater(const vector<T*>& units, int target, vector<int>& bestIdx) {
    int n = units.size();
    int minSum = INT_MAX;
    vector<int> best;
    for (int mask = 1; mask < (1 << n); ++mask) {
        int sum = 0;
        vector<int> idx;
        for (int i = 0; i < n; ++i)
            if (mask & (1 << i)) {
                sum += units[i]->getAttackScore();
                idx.push_back(i);
            }
        if (sum > target && sum < minSum) {
            minSum = sum;
            best = idx;
        }
    }
    if (!best.empty()) {
        bestIdx = best;
        return true;
    }
    return false;
}

// Helper: loại bỏ các unit theo index ra khỏi vector (từ lớn về nhỏ)
template<typename T>
static vector<T*> removeUnitsByIdx(vector<T*>& units, const vector<int>& idx) {
    vector<T*> removed;
    set<int> idxSet(idx.begin(), idx.end());
    for (int i = (int)units.size() - 1; i >= 0; --i) {
        if (idxSet.count(i)) {
            removed.push_back(units[i]);
            units.erase(units.begin() + i);
        }
    }
    return removed;
}

LiberationArmy::LiberationArmy(const Unit** unitArray, int size, BattleField* battleField)
    : Army(unitArray, size, "LiberationArmy", battleField) {}

// Helper: cập nhật LF, EXP từ unitList
void LiberationArmy::updateStatsFromUnitList() {
    int lf = 0, exp = 0;
    for (auto u : unitList->getUnits()) {
        if (auto v = dynamic_cast<Vehicle*>(u)) lf += v->getAttackScore();
        else if (auto inf = dynamic_cast<Infantry*>(u)) exp += inf->getAttackScore();
    }
    LF = max(0, min(1000, lf));
    EXP = max(0, min(500, exp));
}

// Helper: absorb (tịch thu) đơn vị của đối phương (nếu cùng loại thì cộng quantity/weight)
void LiberationArmy::absorbUnits(const vector<Unit*>& unitsToAdd) {
    for (auto u : unitsToAdd) {
        bool found = false;
        for (auto myU : unitList->getUnits()) {
            if (typeid(*u) == typeid(*myU) && u->getCurrentPosition().str() == myU->getCurrentPosition().str()) {
                myU->setQuantity(myU->getQuantity() + u->getQuantity());
                myU->setWeight(myU->getWeight() + u->getWeight());
                found = true;
                break;
            }
        }
        if (!found) unitList->insert(u);
    }
    updateStatsFromUnitList();
}

// Helper: giảm 10% weight mỗi unit
void LiberationArmy::lose10PercentWeight() {
    for (auto u : unitList->getUnits()) {
        int w = u->getWeight();
        u->setWeight(max(1, w - static_cast<int>(round(w * 0.1))));
    }
    updateStatsFromUnitList();
}

// Helper: giảm 10% quantity mỗi unit
void LiberationArmy::lose10PercentQuantity() {
    for (auto u : unitList->getUnits()) {
        int q = u->getQuantity();
        u->setQuantity(max(1, q - static_cast<int>(round(q * 0.1))));
    }
    updateStatsFromUnitList();
}

// Helper: chi viện - tăng quantity lên số Fibonacci gần nhất
void LiberationArmy::reinforceFibo() {
    for (auto u : unitList->getUnits()) {
        u->setQuantity(nearestFibo(u->getQuantity()));
    }
    updateStatsFromUnitList();
}

void LiberationArmy::fight(Army* enemy, bool defense) {
    if (!enemy) return;

    if (!defense) {
        // Attack
        int attackLF = static_cast<int>(round(LF * 1.5));
        int attackEXP = static_cast<int>(round(EXP * 1.5));
        auto myInf = getInfantries(unitList);
        auto myVeh = getVehicles(unitList);

        vector<int> infIdx, vehIdx;
        bool foundInf = findMinSubsetSumGreater(myInf, enemy->getEXP(), infIdx);
        bool foundVeh = findMinSubsetSumGreater(myVeh, enemy->getLF(), vehIdx);

        if (foundInf && foundVeh) {
            // Thắng: xoá tổ hợp, absorb đơn vị đối phương
            vector<Infantry*> removedInf = removeUnitsByIdx(myInf, infIdx);
            vector<Vehicle*> removedVeh = removeUnitsByIdx(myVeh, vehIdx);
            for (auto u : removedInf) unitList->removeUnit(u);
            for (auto u : removedVeh) unitList->removeUnit(u);
            // absorb đơn vị đối phương
            absorbUnits(enemy->getUnitList()->getUnits());
        } else if (foundInf || foundVeh) {
            // Hòa, kiểm tra ưu thế
            if ((foundInf && attackLF > enemy->getLF()) || (foundVeh && attackEXP > enemy->getEXP())) {
                // Thắng, xoá tổ hợp thoả mãn và các đơn vị còn lại của loại còn lại
                if (foundInf) {
                    vector<Infantry*> removedInf = removeUnitsByIdx(myInf, infIdx);
                    for (auto u : removedInf) unitList->removeUnit(u);
                    for (auto v : myVeh) unitList->removeUnit(v);
                } else {
                    vector<Vehicle*> removedVeh = removeUnitsByIdx(myVeh, vehIdx);
                    for (auto u : removedVeh) unitList->removeUnit(u);
                    for (auto inf : myInf) unitList->removeUnit(inf);
                }
                absorbUnits(enemy->getUnitList()->getUnits());
            } else {
                // Không giao tranh
                lose10PercentWeight();
            }
        } else {
            // Không giao tranh
            lose10PercentWeight();
        }
        updateStatsFromUnitList();
    } else {
        // Defense
        int defenseLF = static_cast<int>(round(LF * 1.3));
        int defenseEXP = static_cast<int>(round(EXP * 1.3));

        if (defenseLF >= enemy->getLF() && defenseEXP >= enemy->getEXP()) {
            // Thắng, không làm gì
        } else if (defenseLF < enemy->getLF() && defenseEXP < enemy->getEXP()) {
            // Cả 2 nhỏ hơn, chi viện
            reinforceFibo();
            defenseLF = static_cast<int>(round(LF * 1.3));
            defenseEXP = static_cast<int>(round(EXP * 1.3));
            if (defenseLF >= enemy->getLF() && defenseEXP >= enemy->getEXP()) {
                // Đủ mạnh sau chi viện
            } else {
                // Vẫn yếu hơn, mất 10% quantity mỗi đơn vị
                lose10PercentQuantity();
            }
        } else {
            // Mất 10% quantity mỗi đơn vị
            lose10PercentQuantity();
        }
        updateStatsFromUnitList();
    }
}

string LiberationArmy::str() const {
    string battlefieldStr = battleField ? "BattleField(" + to_string(battleField->getRows()) + "," + to_string(battleField->getCols()) + ")" : "None";
    return "LiberationArmy[name=" + name + ",LF=" + to_string(LF) +
           ",EXP=" + to_string(EXP) + ",unitList=" + unitList->str() +
           ",battleField=" + battlefieldStr + "]";
}

ARVN::ARVN(const Unit** unitArray, int size, BattleField* battleField)
        : Army(unitArray, size, "ARVN", battleField) {}

void ARVN::updateStatsFromUnitList(ARVN* army){
    int lf = 0, exp = 0;
    for (auto u : army->unitList->getUnits()) {
        if (auto v = dynamic_cast<Vehicle*>(u)) lf += v->getAttackScore();
        else if (auto inf = dynamic_cast<Infantry*>(u)) exp += inf->getAttackScore();
    }
    army->LF = std::max(0, std::min(1000, lf));
    army->EXP = std::max(0, std::min(500, exp));
}

void ARVN::fight(Army* enemy, bool defense = false) {
        if (!enemy) return;

        if (!defense) {
            // Chế độ tấn công: ARVN luôn thất bại trước LiberationArmy ở thế phòng thủ
            vector<Unit*> unitsToRemove;
            for (auto u : unitList->getUnits()) {
                int q = u->getQuantity();
                int newQuantity = std::max(1, q - static_cast<int>(std::round(q * 0.2))); // Giảm 20% số lượng
                if (newQuantity <= 1) {
                    unitsToRemove.push_back(u);
                } else {
                    u->setQuantity(newQuantity);
                }
            }
            // Xóa các đơn vị có quantity == 1
            for (auto u : unitsToRemove) {
                unitList->removeUnit(u);
            }
            updateStatsFromUnitList(this);
        } else {
        // ARVN phòng thủ, LiberationArmy tấn công
        // Nếu bị tịch thu: xoá hết, còn lại thì mỗi unit bị mất 20% weight
        bool confiscated = false;
        // Quy tắc bị tịch thu: nếu Army này bị gọi fight với defense=true và sau đó unitList bị clear bởi LiberationArmy
        // Ở đây, giả sử nếu unitList trống sau enemy tịch thu thì bị tịch thu, còn lại thì chỉ mất weight
        if (unitList->getUnits().empty()) {
            confiscated = true;
        }
        if (!confiscated) {
            for (auto u : unitList->getUnits()) {
                int w = u->getWeight();
                int newW = std::max(1, w - static_cast<int>(std::round(w * 0.2)));
                u->setWeight(newW);
            }
            updateStatsFromUnitList(this);
        }
    }
}

string ARVN::str() const {
    stringstream ss;
    ss << "ARVN[name=" << name
        << ",LF=" << LF
        << ",EXP=" << EXP
        << "," << unitList->str()
        << "," << (battleField ? "BattleField" : "NULL")
        << "]";
    return ss.str();
}

UnitList::UnitList(int capacity) : head(nullptr), tail(nullptr), capacity(capacity), 
                           count_vehicle(0), count_infantry(0), size(0) {}

UnitList::~UnitList() {
    Node* current = head;
    while (current) {
        Node* next = current->next;
        delete current;
        current = next;
    }
}

bool UnitList::insert(Unit* unit) {
    if (!unit || size >= capacity) return false;

    // Kiểm tra đơn vị đã tồn tại
    Node* current = head;
    while (current) {
        if (typeid(*unit) == typeid(*current->unit) && 
            unit->getCurrentPosition().str() == current->unit->getCurrentPosition().str()) {
            current->unit->setQuantity(current->unit->getQuantity() + unit->getQuantity());
            return true;
        }
        current = current->next;
    }

    // Thêm đơn vị mới
    Node* newNode = new Node(unit);
    if (dynamic_cast<Vehicle*>(unit)) {
        // Thêm Vehicle vào cuối
        if (!head) {
            head = tail = newNode;
        } else {
            tail->next = newNode;
            tail = newNode;
        }
        ++count_vehicle;
    } else if (dynamic_cast<Infantry*>(unit)) {
        // Thêm Infantry vào đầu
        newNode->next = head;
        head = newNode;
        if (!tail) tail = newNode;
        ++count_infantry;
    } else {
        delete newNode;
        return false; // Không phải Vehicle hoặc Infantry
    }
    ++size;
    return true;
}

bool UnitList::isContain(VehicleType vehicleType) {
    Node* p = head;
    while (p) {
        auto v = dynamic_cast<Vehicle*>(p->unit);
        if (v && v->getVehicleType() == vehicleType) return true;
        p = p->next;
    }
    return false;
}

bool UnitList::isContain(InfantryType infantryType) {
    Node* p = head;
    while (p) {
        auto i = dynamic_cast<Infantry*>(p->unit);
        if (i && i->getInfantryType() == infantryType) return true;
        p = p->next;
    }
    return false;
}

string UnitList::str() const {
    string unit_list;
    Node* current = head;
    while (current) {
        unit_list += current->unit->str();
        if (current->next) unit_list += ",";
        current = current->next;
    }
    return "UnitList[count_vehicle=" + to_string(count_vehicle) + 
            ";count_infantry=" + to_string(count_infantry) + ";" + unit_list + "]";
}

vector<Unit*> UnitList::getUnits() const {
    vector<Unit*> units;
    Node* current = head;
    while (current) {
        units.push_back(current->unit);
        current = current->next;
    }
    return units;
}

bool UnitList::removeUnit(Unit* unit) {
    Node* current = head;
    Node* prev = nullptr;
    while (current) {
        if (current->unit == unit) {
            if (dynamic_cast<Vehicle*>(unit)) --count_vehicle;
            else if (dynamic_cast<Infantry*>(unit)) --count_infantry;
            if (prev) {
                prev->next = current->next;
            } else {
                head = current->next;
            }
            if (current == tail) tail = prev;
            delete current;
            --size;
            return true;
        }
        prev = current;
        current = current->next;
    }
    return false;
}

void UnitList::removeUnits() {
    Node* current = head;
    Node* prev = nullptr;
    while (current) {
        Unit* unit = current->unit;
        if (unit && unit->getQuantity() <= 0) {
            Node* toDelete = current;
            if (prev) {
                prev->next = current->next;
            } else {
                head = current->next;
            }
            current = current->next;
            delete toDelete->unit;
            delete toDelete;
            continue;
        }
        prev = current;
        current = current->next;
    }
}

TerrainElement::TerrainElement() {}
TerrainElement::~TerrainElement() {}
int TerrainElement::distance(const Position& p1, const Position& p2) {
    int dx = p1.getRow() - p2.getRow();
    int dy = p1.getCol() - p2.getCol();
    if (dx < 0) dx = -dx;
    if (dy < 0) dy = -dy;
    return dx + dy;
}

void Road::getEffect(Army* army) {}

void Forest::getEffect(Army* army) {

        bool isLiberation = dynamic_cast<LiberationArmy*>(army) != nullptr;
        bool isARVN = dynamic_cast<ARVN*>(army) != nullptr;
        if (!isLiberation && !isARVN) return;

        std::vector<Unit*> units = army->getUnitList()->getUnits();
        int infantryRadius = isLiberation ? 2 : 4;
        double expPercent = isLiberation ? 0.3 : 0.2;
        double lfPercent = isLiberation ? 0.1 : 0.05;

        int expDelta = 0, lfDelta = 0;

        for (auto u : units) {
            // Bộ binh được cộng EXP
            if (auto inf = dynamic_cast<Infantry*>(u)) {
                if (TerrainElement::distance(pos, inf->getCurrentPosition()) <= infantryRadius) {
                    int delta = static_cast<int>(inf->getAttackScore() * expPercent + 0.5);
                    expDelta += delta;
                }
            }
            // Phương tiện bị trừ LF
            if (auto veh = dynamic_cast<Vehicle*>(u)) {
                if (TerrainElement::distance(pos, veh->getCurrentPosition()) <= infantryRadius) {
                    int delta = static_cast<int>(veh->getAttackScore() * lfPercent + 0.5);
                    lfDelta += delta;
                }
            }
        }

        army->setEXP(army->getEXP() + expDelta);
        army->setLF(army->getLF() - lfDelta);
        // Giới hạn giá trị
        if (army->getEXP() > 500) army->setEXP(500);
        if (army->getEXP() < 0) army->setEXP(0);
        if (army->getLF() > 1000) army->setLF(1000);
        if (army->getLF() < 0) army->setLF(0);
    }

void River::getEffect(Army* army) {
    std::vector<Unit*> units = army->getUnitList()->getUnits();
    for (auto u : units) {
        Infantry* inf = dynamic_cast<Infantry*>(u);
        if (inf && TerrainElement::distance(pos, inf->getCurrentPosition()) <= 2) {
            inf->multiplyAttackScore(0.9); // giảm 10%
        }
    }
}

void Urban::getEffect(Army* army) {
    // Giải phóng: SPECIALFORCES hoặc REGULARINFANTRY, bán kính 5, tăng (2*attackScore)/D
    if (auto liberation = dynamic_cast<LiberationArmy*>(army)) {
        std::vector<Unit*> units = army->getUnitList()->getUnits();
        for (auto u : units) {
            Infantry* inf = dynamic_cast<Infantry*>(u);
            if (inf) {
                InfantryType type = inf->getInfantryType();
                if ((type == SPECIALFORCES || type == REGULARINFANTRY)) {
                    int D = TerrainElement::distance(pos, inf->getCurrentPosition());
                    if (D > 0 && D <= 5) {
                        int baseScore = inf->getAttackScore();
                        int delta = static_cast<int>((2.0 * baseScore) / D + 0.5); // làm tròn
                        inf->multiplyAttackScore(delta);
                    }
                }
            }
            // Pháo binh bị giảm 50% attackScore nếu trong bán kính 2
            Vehicle* veh = dynamic_cast<Vehicle*>(u);
            if (veh && veh->getVehicleType() == ARTILLERY) {
                int D = TerrainElement::distance(pos, veh->getCurrentPosition());
                if (D <= 2) {
                    veh->multiplyAttackScore(0.5);
                }
            }
        }
    }
    // CQ Sài Gòn: REGULARINFANTRY, bán kính 3, tăng (3*attackScore)/(2*D)
    else if (auto arvn = dynamic_cast<ARVN*>(army)) {
        std::vector<Unit*> units = army->getUnitList()->getUnits();
        for (auto u : units) {
            Infantry* inf = dynamic_cast<Infantry*>(u);
            if (inf && inf->getInfantryType() == REGULARINFANTRY) {
                int D = TerrainElement::distance(pos, inf->getCurrentPosition());
                if (D > 0 && D <= 3) {
                    int baseScore = inf->getAttackScore();
                    int delta = static_cast<int>((3.0 * baseScore) / (2 * D) + 0.5);
                    inf->addAttackScoreModifier(delta);
                }
            }
            // Phương tiện chiến đấu không bị ảnh hưởng, KHÔNG làm gì
        }
    }
}

void Fortification::getEffect(Army* army) {
    // Quân Giải phóng: tất cả đơn vị trong bán kính 2 đều bị trừ 20% attackScore
    if (dynamic_cast<LiberationArmy*>(army)) {
        std::vector<Unit*> units = army->getUnitList()->getUnits();
        for (auto u : units) {
            if (TerrainElement::distance(pos, u->getCurrentPosition()) <= 2) {
                // Trừ 20% attackScore (nhân 0.8)
                u->multiplyAttackScore(0.8);
            }
        }
    }
    // Quân đội CQ Sài Gòn: các đơn vị trong bán kính 2 được cộng 20% attackScore
    else if (dynamic_cast<ARVN*>(army)) {
        std::vector<Unit*> units = army->getUnitList()->getUnits();
        for (auto u : units) {
            if (TerrainElement::distance(pos, u->getCurrentPosition()) <= 2) {
                // Cộng 20% attackScore (nhân 1.2)
                u->multiplyAttackScore(1.2);
            }
        }
    }
}

void SpecialZone::getEffect(Army* army) {
    // Mọi đơn vị trong bán kính 1 đều bị giảm attackScore xuống 0
    std::vector<Unit*> units = army->getUnitList()->getUnits();
    for (auto u : units) {
        if (TerrainElement::distance(pos, u->getCurrentPosition()) <= 1) {
            u->setAttackScoreMultiplier(0); // hoặc override getAttackScore trả về 0 nếu multiplier = 0
            u->resetAttackScoreModifier();
        }
    }
}

BattleField::BattleField(int n_rows, int n_cols,
    std::vector<Position*> arrayForest,
    std::vector<Position*> arrayRiver,
    std::vector<Position*> arrayFortification,
    std::vector<Position*> arrayUrban,
    std::vector<Position*> arraySpecialZone)
    : n_rows(n_rows), n_cols(n_cols)
{
    terrain.resize(n_rows, std::vector<TerrainElement*>(n_cols, nullptr));
    for (int i = 0; i < n_rows; ++i) {
        for (int j = 0; j < n_cols; ++j) {
            Position pos(i, j);
            if (contains(arrayForest, pos))
                terrain[i][j] = new Forest(pos);
            else if (contains(arrayRiver, pos))
                terrain[i][j] = new River(pos);
            else if (contains(arrayFortification, pos))
                terrain[i][j] = new Fortification(pos);
            else if (contains(arrayUrban, pos))
                terrain[i][j] = new Urban(pos);
            else if (contains(arraySpecialZone, pos))
                terrain[i][j] = new SpecialZone(pos);
            // else: terrain[i][j] = nullptr; (ngầm định)
        }
    }
}

BattleField::~BattleField() {
    for (int i = 0; i < n_rows; ++i)
        for (int j = 0; j < n_cols; ++j)
            delete terrain[i][j];
}

void BattleField::applyTerrainEffects() const {
    // Áp dụng hiệu ứng cho LiberationArmy
    if (liberationArmy) {
        for (int row = 0; row < n_rows; ++row) {
            for (int col = 0; col < n_cols; ++col) {
                applyTerrainEffects(row, col, liberationArmy);
            }
        }
    }
    // Áp dụng hiệu ứng cho ARVN
    if (arvn) {
        for (int row = 0; row < n_rows; ++row) {
            for (int col = 0; col < n_cols; ++col) {
                applyTerrainEffects(row, col, arvn);
            }
        }
    }
}

// Hàm này giữ nguyên như bạn đã có:
void BattleField::applyTerrainEffects(int row, int col, Army* army) const {
    if (row < 0 || row >= n_rows || col < 0 || col >= n_cols || !army) return;
    void* current = army->getUnitList()->getHead();
    while (current) {
        Unit* unit = army->getUnitList()->getUnit(current);
        if (unit && unit->getQuantity() > 0) {
            Position unitPos = unit->getCurrentPosition();
            double distance = sqrt(pow(unitPos.getRow() - row, 2) + pow(unitPos.getCol() - col, 2));
            // Áp dụng hiệu ứng dựa trên loại địa hình và bán kính
            if (dynamic_cast<Forest*>(terrain[row][col]) && distance <= 2.0) {
                // Giảm 20% LF cho phương tiện
                army->setLF(static_cast<int>(army->getLF() * 0.8));
            } else if (dynamic_cast<River*>(terrain[row][col]) && distance <= 2.0) {
                // Giảm 10% EXP
                army->setEXP(static_cast<int>(army->getEXP() * 0.9));
            } else if (dynamic_cast<Urban*>(terrain[row][col]) && distance <= 2.0) {
                // Tăng 10% LF
                army->setLF(static_cast<int>(army->getLF() * 1.1));
            } else if (dynamic_cast<Fortification*>(terrain[row][col]) && distance <= 2.0) {
                // Tăng 20% LF, 10% EXP
                army->setLF(static_cast<int>(army->getLF() * 1.2));
                army->setEXP(static_cast<int>(army->getEXP() * 1.1));
            } else if (dynamic_cast<SpecialZone*>(terrain[row][col]) && distance <= 1.0) {
                // Tăng 50 LF và EXP
                army->setLF(army->getLF() + 50);
                army->setEXP(army->getEXP() + 50);
            }
        }
        current = army->getUnitList()->getNext(current);
    }
}

string BattleField::str() const {
    std::ostringstream oss;
    // Format: BattleField[attr_name=<n_rows>,<n_cols>]
    oss << "BattleField[attr_name=" << n_rows << "," << n_cols << "]";
    return oss.str();
}

string Configuration::trim(const std::string& s) {
    size_t l = s.find_first_not_of(" \t\r\n");
    size_t r = s.find_last_not_of(" \t\r\n");
    if (l == std::string::npos || r == std::string::npos) return "";
    return s.substr(l, r - l + 1);
}

Position* Configuration::parsePosition(const std::string& s) {
    int x = 0, y = 0;
    size_t l = s.find('(');
    size_t c = s.find(',');
    size_t r = s.find(')');
    if (l == std::string::npos || c == std::string::npos || r == std::string::npos) return nullptr;
    x = std::stoi(s.substr(l + 1, c - l - 1));
    y = std::stoi(s.substr(c + 1, r - c - 1));
    return new Position(x, y);
}

void Configuration::parsePositionArray(const std::string& line, std::vector<Position*>& arr) {
        size_t l = line.find('[');
        size_t r = line.find(']');
        if (l == std::string::npos || r == std::string::npos || r <= l) return;
        std::string content = line.substr(l + 1, r - l - 1);
        std::stringstream ss(content);
        std::string token;
        while (std::getline(ss, token, ')')) {
            size_t lp = token.find('(');
            if (lp == std::string::npos) continue;
            token = token.substr(lp) + ')';
            token = trim(token);
            Position* p = parsePosition(token);
            if (p) arr.push_back(p);
        }
}

void Configuration::parseUnitArray(const std::string& line, std::vector<Unit*>& liber, std::vector<Unit*>& arvn) {
        size_t l = line.find('[');
        size_t r = line.find(']');
        if (l == std::string::npos || r == std::string::npos || r <= l) return;
        std::string content = line.substr(l + 1, r - l - 1);
        size_t i = 0, n = content.size();
        while (i < n) {
            // Tìm tên loại
            size_t startType = i;
            while (i < n && std::isalpha(content[i])) ++i;
            std::string type = content.substr(startType, i - startType);
            while (i < n && std::isspace(content[i])) ++i;
            if (i >= n || content[i] != '(') break;
            ++i; // bỏ '('
            // Lấy các tham số, format: q,w,(x,y),side
            // q
            size_t start = i;
            while (i < n && content[i] != ',') ++i;
            int q = std::stoi(content.substr(start, i - start));
            ++i; // bỏ ','
            // w
            start = i;
            while (i < n && content[i] != ',') ++i;
            int w = std::stoi(content.substr(start, i - start));
            ++i; // bỏ ','
            // (x,y)
            while (i < n && content[i] != '(') ++i;
            size_t pos_l = i;
            while (i < n && content[i] != ')') ++i;
            size_t pos_r = i;
            std::string pos_str = content.substr(pos_l, pos_r - pos_l + 1);
            Position* pos = parsePosition(pos_str);
            ++i; // bỏ ')'
            if (i < n && content[i] == ',') ++i;
            // side
            start = i;
            while (i < n && std::isdigit(content[i])) ++i;
            int side = std::stoi(content.substr(start, i - start));
            // bỏ ')', ',' nếu còn
            while (i < n && (content[i] == ')' || content[i] == ',' || std::isspace(content[i]))) ++i;

            // Tạo Unit
            Unit* unit = nullptr;
            if (type == "TANK") unit = new Vehicle(q, w, *pos, TANK);
            else if (type == "TRUCK") unit = new Vehicle(q, w, *pos, TRUCK);
            else if (type == "MORTAR") unit = new Vehicle(q, w, *pos, MORTAR);
            else if (type == "ANTIAIRCRAFT") unit = new Vehicle(q, w, *pos, ANTIAIRCRAFT);
            else if (type == "ARMOREDCAR") unit = new Vehicle(q, w, *pos, ARMOREDCAR);
            else if (type == "APC") unit = new Vehicle(q, w, *pos, APC);
            else if (type == "ARTILLERY") unit = new Vehicle(q, w, *pos, ARTILLERY);
            else if (type == "SNIPER") unit = new Infantry(q, w, *pos, SNIPER);
            else if (type == "ANTIAIRCRAFTSQUAD") unit = new Infantry(q, w, *pos, ANTIAIRCRAFTSQUAD);
            else if (type == "MORTARSQUAD") unit = new Infantry(q, w, *pos, MORTARSQUAD);
            else if (type == "ENGINEER") unit = new Infantry(q, w, *pos, ENGINEER);
            else if (type == "SPECIALFORCES") unit = new Infantry(q, w, *pos, SPECIALFORCES);
            else if (type == "REGULARINFANTRY") unit = new Infantry(q, w, *pos, REGULARINFANTRY);
            delete pos;
            if (unit) {
                if (side == 0) liber.push_back(unit);
                else arvn.push_back(unit);
            }
        }
}

int Configuration::parseInt(const std::string& line) {
        size_t eq = line.find('=');
        if (eq == std::string::npos) return 0;
        return std::stoi(trim(line.substr(eq + 1)));
}

Configuration::Configuration(const std::string& filepath) {
    std::ifstream fin(filepath.c_str());
    if (!fin) {
        std::cerr << "Cannot open config file!\n";
        std::exit(1);
    }
    std::string line;
    while (std::getline(fin, line)) {
        if (line.find("NUM_ROWS=") == 0)
            num_rows = parseInt(line);
        else if (line.find("NUM_COLS=") == 0)
            num_cols = parseInt(line);
        else if (line.find("ARRAY_FOREST=") == 0)
            parsePositionArray(line, arrayForest);
        else if (line.find("ARRAY_RIVER=") == 0)
            parsePositionArray(line, arrayRiver);
        else if (line.find("ARRAY_FORTIFICATION=") == 0)
            parsePositionArray(line, arrayFortification);
        else if (line.find("ARRAY_URBAN=") == 0)
            parsePositionArray(line, arrayUrban);
        else if (line.find("ARRAY_SPECIAL_ZONE=") == 0)
            parsePositionArray(line, arraySpecialZone);
        else if (line.find("UNIT_LIST=") == 0)
            parseUnitArray(line, liberationUnits, ARVNUnits);
        else if (line.find("EVENT_CODE=") == 0) {
            int code = parseInt(line);
            if (code < 0) eventCode = 0;
            else eventCode = code % 100;
        }
    }
    fin.close();
}

Configuration::~Configuration() {
    for (auto p : arrayForest) delete p;
    for (auto p : arrayRiver) delete p;
    for (auto p : arrayFortification) delete p;
    for (auto p : arrayUrban) delete p;
    for (auto p : arraySpecialZone) delete p;
    for (auto u : liberationUnits) delete u;
    for (auto u : ARVNUnits) delete u;
}

string Configuration::str() const {
    std::ostringstream oss;
    oss << "Configuration[\n";
    oss << "num_rows=" << num_rows << "\n";
    oss << "num_cols=" << num_cols << "\n";
    auto arrStr = [](const std::vector<Position*>& arr) {
        std::ostringstream s;
        s << "[";
        for (size_t i = 0; i < arr.size(); ++i) {
            s << "(" << arr[i]->getRow() << "," << arr[i]->getCol() << ")";
            if (i + 1 < arr.size()) s << ",";
        }
        s << "]";
        return s.str();
    };
    oss << "arrayForest=" << arrStr(arrayForest) << "\n";
    oss << "arrayRiver=" << arrStr(arrayRiver) << "\n";
    oss << "arrayFortification=" << arrStr(arrayFortification) << "\n";
    oss << "arrayUrban=" << arrStr(arrayUrban) << "\n";
    oss << "arraySpecialZone=" << arrStr(arraySpecialZone) << "\n";
    // Liberation units
    oss << "liberationUnits=[";
    for (size_t i = 0; i < liberationUnits.size(); ++i) {
        oss << liberationUnits[i]->str();
        if (i + 1 < liberationUnits.size()) oss << ",";
    }
    oss << "]\n";
    // ARVN units
    oss << "ARVNUnits=[";
    for (size_t i = 0; i < ARVNUnits.size(); ++i) {
        oss << ARVNUnits[i]->str();
        if (i + 1 < ARVNUnits.size()) oss << ",";
    }
    oss << "]\n";
    oss << "eventCode=" << eventCode << "\n";
    oss << "]";
    return oss.str();
}

HCMCampaign::HCMCampaign(const string& config_file_path) 
    : config(new Configuration(config_file_path)), battleField(nullptr),
      liberationArmy(nullptr), arvn(nullptr) {
        if (!config || config->getNumRows() <= 0 || config->getNumCols() <= 0) {
            delete config;
            config = nullptr;
            return;
        }

        // Khởi tạo battleField
        battleField = new BattleField(
            config->getNumRows(), config->getNumCols(),
            config->getArrayForest(), config->getArrayRiver(),
            config->getArrayFortification(), config->getArrayUrban(),
            config->getArraySpecialZone()
        );

        // Khởi tạo LiberationArmy
        const std::vector<Unit*>& libUnits = config->getLiberationUnits();
        Unit** libArray = new Unit*[libUnits.size()];
        for (size_t i = 0; i < libUnits.size(); ++i) {
            libArray[i] = libUnits[i]->clone();
        }
        liberationArmy = new LiberationArmy(
            const_cast<const Unit**>(libArray),
            static_cast<int>(libUnits.size()),
            battleField
        );
        for (size_t i = 0; i < libUnits.size(); ++i) {
            delete libArray[i];
        }
        delete[] libArray;

        // Khởi tạo ARVN
        const std::vector<Unit*>& arvnUnits = config->getARVNUnits();
        Unit** arvnArray = new Unit*[arvnUnits.size()];
        for (size_t i = 0; i < arvnUnits.size(); ++i) {
            arvnArray[i] = arvnUnits[i]->clone();
        }
        arvn = new ARVN(
            const_cast<const Unit**>(arvnArray),
            static_cast<int>(arvnUnits.size()),
            battleField
        );
        for (size_t i = 0; i < arvnUnits.size(); ++i) {
            delete arvnArray[i];
        }
        delete[] arvnArray;
    }

HCMCampaign::~HCMCampaign() {
    delete liberationArmy;
    delete arvn;
    delete battleField;
    delete config;
}

void HCMCampaign::run() {
    if (battleField && liberationArmy && arvn) {
        // Áp dụng hiệu ứng địa hình trước
        battleField->applyTerrainEffects();
        // Thực hiện giao tranh
        battleField->battle();
    }
        if (!liberationArmy || !arvn || !battleField || !liberationArmy->getUnitList() || !arvn->getUnitList()) return;
    if (!liberationArmy->getUnitList() || !arvn->getUnitList()) return;
    int maxRounds = 10;
    for (int round = 0; round < maxRounds; ++round) {
        void* current = liberationArmy->getUnitList()->getHead();
        while (current) {
            Unit* unit = liberationArmy->getUnitList()->getUnit(current);
            if (unit->getQuantity() > 0) { // Chỉ áp dụng hiệu ứng cho đơn vị còn sống
                Position pos = unit->getCurrentPosition();
                battleField->applyTerrainEffects(pos.getRow(), pos.getCol(), liberationArmy);
            }
            current = liberationArmy->getUnitList()->getNext(current);
        }
        current = arvn->getUnitList()->getHead();
        while (current) {
            Unit* unit = arvn->getUnitList()->getUnit(current);
            if (unit->getQuantity() > 0) { // Chỉ áp dụng hiệu ứng cho đơn vị còn sống
                Position pos = unit->getCurrentPosition();
                battleField->applyTerrainEffects(pos.getRow(), pos.getCol(), arvn);
            }
            current = arvn->getUnitList()->getNext(current);
        }
        liberationArmy->fight(arvn, false);
        arvn->fight(liberationArmy, true);

    liberationArmy->getUnitList()->removeUnits();
    arvn->getUnitList()->removeUnits();

        bool libEmpty = true, arvnEmpty = true;
        current = liberationArmy->getUnitList()->getHead();
        while (current) {
            if (liberationArmy->getUnitList()->getUnit(current)->getQuantity() > 0) {
                libEmpty = false;
                break;
            }
            current = liberationArmy->getUnitList()->getNext(current);
        }
        current = arvn->getUnitList()->getHead();
        while (current) {
            if (arvn->getUnitList()->getUnit(current)->getQuantity() > 0) {
                arvnEmpty = false;
                break;
            }
            current = arvn->getUnitList()->getNext(current);
        }
        if (libEmpty || arvnEmpty) break;
    }
}

std::string HCMCampaign::printResult() const {
    std::ostringstream oss;
    if (liberationArmy && arvn) {
        oss << "LIBERATIONARMY[LF=" << liberationArmy->getLF()
            << ",EXP=" << liberationArmy->getEXP() << "]"
            << "-ARVN[LF=" << arvn->getLF()
            << ",EXP=" << arvn->getEXP() << "]";
    }
    return oss.str();
}

////////////////////////////////////////////////
/// END OF STUDENT'S ANSWER
////////////////////////////////////////////////