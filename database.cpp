#include "database.h"
#include <fstream>



void Database::Add(const Date& date, const std::string& event){
    if(!ALL_DATA_SET.count(date) || !ALL_DATA_SET.at(date).count(event)){
        ALL_DATA[date].push_back(event);
        ALL_DATA_SET[date].insert(event);
    }//добавляем событие,если оно отсутствует

}

bool Database::IsHere(const Date& date, const std::string& event){
    auto it = std::find(std::begin(ALL_DATA[date]),std::end(ALL_DATA[date]),event);
    if(it != std::end(ALL_DATA[date]))//элемент есть
        return false;
    return true;//элемента нет
}

std::string Database::Last(const Date& date) const{

    if(ALL_DATA.count(date))//если дата имеется в словаре(проверить случай поиска последнего события после его удаления)
        return date.ToString() + ' ' + ALL_DATA.at(date)[ALL_DATA.at(date).size() - 1];
    else{
        auto it = ALL_DATA.upper_bound(date);
        if(it == ALL_DATA.begin() || ALL_DATA.size() == 0)
            return "No entries";
        else{
            return (prev(it))->first.ToString() + ' ' + *(--(prev(it)->second).end());
        }
    }

}


std::string Database::ToStringDB() const{
    std::string result = "";
    for(const auto& item : ALL_DATA){
        result += ToStringVector(item.first,item.first.ToString());
    }
    return result;
}

std::string Database::ToStringVector(const Date& date,std::string nums) const{
    std::string result = "";
    int index = 0;
    if(ALL_DATA.count(date)){
        for(const auto& item : ALL_DATA.at(date)){
            result += nums + ' ' + item;
            if(index != ALL_DATA.at(date).size() - 1)
                result += '\n';
            index++;
        }
    }
    result += "\n";
    return result;
}

void Database::Print(std::ostream& output) const{
    output << this->ToStringDB();
}