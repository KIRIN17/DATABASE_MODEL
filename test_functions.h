template<class T>
ostream &operator<<(ostream &os, const set<T> &s) {
    os << "{";
    bool first = true;
    for (const auto &x : s) {
        if (!first) {
            os << ", ";
        }
        first = false;
        os << x;
    }
    return os << "}";
}

template<class K, class V>
ostream &operator<<(ostream &os, const map<K, V> &m) {
    os << "{";
    bool first = true;
    for (const auto &kv : m) {
        if (!first) {
            os << ", ";
        }
        first = false;
        os << kv.first << ": " << kv.second;
    }
    return os << "}";
}

template<class K, class V>
ostream &operator<<(ostream &os, const pair<K, V> &m) {

    for (const auto &kv : m) {
        os << kv.first << " " << kv.second;
    }
    return os;
}

//template <typename T>
//std::ostream& operator << (std::ostream& out, const std::vector<T> v){
//    out << "{ ";
//    for(const T& elem : v)
//        out << elem << ' ';
//    out << "}";
//    return out;
//}
struct Entry {
    Entry(Date date, string s) {
        date_ = date;
        s_ = s;
    }

    Date date_ = {0, 0, 0};
    string s_;
};

bool operator!=(const string &lhs, const Entry &rhs) {
    return lhs != rhs.s_;//make_tuple(lhs.date_,lhs.s_) != make_tuple(rhs.date_,rhs.s_);
}

bool operator!=(const Entry &rhs, const string &lhs) {
    return lhs != rhs.s_;//make_tuple(lhs.date_,lhs.s_) != make_tuple(rhs.date_,rhs.s_);
}

ostream &operator<<(ostream &out, const Entry &e) {
    return out << e.date_ << ' ' << e.s_ << endl;
}

template<class T, class U>
void AssertEqual(const T &t, const U &u,
                 const string &hint) {
    if (t != u) {
        ostringstream os;
        os << "Assertion failed: " << t << " != " << u
           << " hint: " << hint;
        throw runtime_error(os.str());
    }
}

inline void Assert(bool b, const string &hint) {
    AssertEqual(b, true, hint);
}

std::string ParseEvent(std::istream &is) {
    std::string res;
    is >> std::ws;
    std::getline(is, res);
    return res;
}

class TestRunner {
public:
    template<class TestFunc>
    void RunTest(TestFunc func, const string &test_name);

    ~TestRunner();

private:
    int fail_count = 0;
};

template<class TestFunc>
void TestRunner::RunTest(TestFunc func, const string &test_name) {
    try {
        func();
        cerr << test_name << " OK" << endl;
    } catch (runtime_error &e) {
        ++fail_count;
        cerr << test_name << " fail: " << e.what() << endl;
    }
}

TestRunner::~TestRunner() {
    if (fail_count > 0) {
        cerr << fail_count << " unit tests failed. Terminate" << endl;
        exit(1);
    }
}

int DoRemove(Database &db, const string &str) {
    istringstream is(str);
    auto condition = ParseCondition(is);
    auto predicate = [condition](const Date &date, const string &event) {
        return condition->Evaluate(date, event);
    };
    return db.RemoveIf(predicate);
}

string DoFind(Database &db, const string &str) {
    istringstream is(str);
    auto condition = ParseCondition(is);
    auto predicate = [condition](const Date &date, const string &event) {
        return condition->Evaluate(date, event);
    };
    const auto entries = db.FindIf(predicate);
    ostringstream os;
    for (const auto &entry : entries) {
        os << entry << endl;
    }
    os << entries.size();
    return os.str();
}

void TestDbAdd() {
    {
        Database db;
        db.Add({2017, 1, 1}, "new year");
        db.Add({2017, 1, 7}, "xmas");
        ostringstream out;
        db.Print(out);
        AssertEqual("2017-01-01 new year\n2017-01-07 xmas\n", out.str(), "straight ordering");
    }
    {
        Database db;
        db.Add({2017, 1, 1}, "new year");
        db.Add({2017, 1, 1}, "holiday");
        ostringstream out;
        db.Print(out);
        AssertEqual("2017-01-01 new year\n2017-01-01 holiday\n", out.str(), "several in one day");
    }
    {
        Database db;
        db.Add({2017, 1, 7}, "xmas");
        db.Add({2017, 1, 1}, "new year");
        ostringstream out;
        db.Print(out);
        AssertEqual("2017-01-01 new year\n2017-01-07 xmas\n", out.str(), "reverse ordering");
    }
    {
        Database db;
        db.Add({2017, 1, 1}, "new year");
        db.Add({2017, 1, 1}, "new year");
        db.Add({2017, 1, 1}, "xmas");
        db.Add({2017, 1, 1}, "new year");
        ostringstream out;
        db.Print(out);
        AssertEqual("2017-01-01 new year\n2017-01-01 xmas\n", out.str(), "uniq adding");
    }
}

void TestDbFind() {
    {
        Database db;
        db.Add({2017, 1, 1}, "new year");
        db.Add({2017, 1, 7}, "xmas");
        AssertEqual("2017-01-01 new year\n1", DoFind(db, "date == 2017-01-01"), "simple find by date");
        AssertEqual("2017-01-01 new year\n2017-01-07 xmas\n2", DoFind(db, "date < 2017-01-31"),
                    "multiple find by date");
        AssertEqual("2017-01-01 new year\n1", DoFind(db, R"(event != "xmas")"), "multiple find by holiday");
    }
    {
        Database db;
        db.Add({2017, 1, 1}, "new year");
        db.Add({2017, 1, 1}, "new year2");
        db.Add({2017, 1, 7}, "xmas");
        AssertEqual("2017-01-01 new year\n2017-01-07 xmas\n2",
                    DoFind(db, R"(date == 2017-01-07 OR event == "new year")"),
                    "complex find or");
        AssertEqual("2017-01-01 new year\n1", DoFind(db, R"(date == 2017-01-01 AND event == "new year")"),
                    "complex find and");
        AssertEqual("0", DoFind(db, R"(date == 2017-01-09 AND event == "new year")"),
                    "complex find and, nothing");
    }
}

void TestDbLast() {
    Database db;
    db.Add({2017, 1, 1}, "new year");
    db.Add({2017, 1, 7}, "xmas");
    {
        try {
            db.Last({2016, 12, 31});
            Assert(false, "last, found no entries");
        } catch (...) {
            Assert(true, "last, found no entries");

        }
    }
    {
        ostringstream os;
        os << db.Last({2017, 1, 2});
        AssertEqual("2017-01-01 new year", os.str(), "greater than date");
    }
    {
        ostringstream os;
        os << db.Last({2017, 1, 1});
        AssertEqual("2017-01-01 new year", os.str(), "eq to date");
    }
    {
        ostringstream os;
        os << db.Last({2017, 1, 10});
        AssertEqual("2017-01-07 xmas", os.str(), "greater than max date");
    }
}

void TestDbRemoveIf() {
    {
        Database db;
        db.Add({2017, 1, 1}, "new year");
        db.Add({2017, 1, 7}, "xmas");
        AssertEqual(0, DoRemove(db, R"(event == "something")"), "Remove nothing");
        AssertEqual(1, DoRemove(db, R"(date == "2017-01-01")"), "Remove by date");
        ostringstream out;
        db.Print(out);
        AssertEqual("2017-01-07 xmas\n", out.str(), "Remove by date, left");
    }
    {
        Database db;
        db.Add({2017, 1, 1}, "new year");
        db.Add({2017, 1, 7}, "xmas");
        AssertEqual(1, DoRemove(db, R"(event == "xmas")"), "Remove by event");
        ostringstream out;
        db.Print(out);
        AssertEqual("2017-01-01 new year\n", out.str(), "Remove by event, left");
    }
    {
        Database db;
        db.Add({2017, 1, 1}, "new year");
        db.Add({2017, 1, 7}, "xmas");
        db.Add({2017, 1, 7}, "new year");
        AssertEqual(2, DoRemove(db, R"(event == "new year")"), "Multiple remove by event");
        ostringstream out;
        db.Print(out);
        AssertEqual("2017-01-07 xmas\n", out.str(), "Multiple remove by event, left");
    }
}

void TestInsertionOrder() {
    {
        Database db;
        db.Add({2017, 1, 1}, "new year");
        db.Add({2017, 1, 7}, "xmas");
        db.Add({2017, 1, 7}, "party");
        db.Add({2017, 1, 7}, "pie");
        ostringstream out;
        db.Print(out);
        AssertEqual("2017-01-01 new year\n2017-01-07 xmas\n2017-01-07 party\n2017-01-07 pie\n", out.str(),
                    "Remove by date, left");
    }

}

void TestsMyCustom() {
    {
        Database db;
        db.Add({2019, 12, 18}, "Kolya");
        db.Add({2020, 1, 15}, "Katya");
        db.Add({2020, 2, 22}, "Riding");
        db.Add({2019, 12, 9}, "Go home");
        db.Add({2019, 12, 9}, "Read");
        ostringstream out;
        db.Print(out);
        AssertEqual("2019-12-09 Go home\n2019-12-09 Read\n2019-12-18 Kolya\n2020-01-15 Katya\n2020-02-22 Riding\n",
                    out.str(), "Order of insertion");
    }

    {
        Database db;
        db.Add({2019, 12, 1}, "a");
        db.Add({2019, 12, 1}, "b");
        db.Add({2019, 12, 2}, "c");
        db.Add({2019, 12, 2}, "d");
        db.Add({2019, 12, 3}, "e");
        ostringstream out;
        db.Print(out);
        AssertEqual("2019-12-01 a\n2019-12-01 b\n2019-12-02 c\n2019-12-02 d\n2019-12-03 e\n", out.str(),
                    "Order of insertion");
    }

    {
        Database db;
        istringstream ss("    a");
        const auto event = ParseEvent(ss);
        db.Add({2019, 12, 1}, event);
        ostringstream out;
        db.Print(out);
        AssertEqual("2019-12-01 a\n", out.str(), "My test 20");
    }

    {
        Database db;
        db.Add({2019, 12, 1}, "a");
        db.Add({2019, 12, 1}, "b");
        db.Add({2019, 12, 2}, "c c");
        db.Add({2019, 12, 2}, "d");
        db.Add({2019, 12, 3}, "e");
        db.Add({2019, 12, 3}, "f");

        ostringstream out;
        db.Print(out);
        AssertEqual("2019-12-01 a\n2019-12-01 b\n2019-12-02 c c\n2019-12-02 d\n2019-12-03 e\n2019-12-03 f\n", out.str(),
                    "My test 2");
        AssertEqual(1, DoRemove(db, R"(event == "c" OR event == "d")"), "My test 3");
        AssertEqual(1, DoRemove(db, R"(event == "e" AND event != "a")"), "My test 4");
        db.Add({2019, 11, 30}, "a");
        AssertEqual("2019-12-03 f\n1", DoFind(db, R"(date >= 2019-12-3)"), "My test 5");
        AssertEqual(Entry({2019, 12, 3}, "f"), db.Last({2019, 12, 4}), " My test 6");

        try {
            db.Last({2019, 11, 3});
        }
        catch (invalid_argument &) {
            cerr << "Test for No entries OK" << endl;
        }

        AssertEqual(Entry({2019, 12, 2}, "c c"), db.Last({2019, 12, 2}), " My test 7");

        AssertEqual(Entry({2019, 12, 3}, "f"), db.Last({2019, 12, 4}), " My test 8");

        db.Add({2019, 12, 3}, "m");
        AssertEqual(Entry({2019, 12, 3}, "m"), db.Last({2019, 12, 3}), " My test 9");

        AssertEqual(1, DoRemove(db, R"(event == "e" AND event != "a" OR event == "m" AND date == 2019-12-3)"),
                    "My test 10");

        ostringstream out2;
        db.Print(out2);
        AssertEqual("2019-11-30 a\n2019-12-01 a\n2019-12-01 b\n2019-12-02 c c\n2019-12-03 f\n", out2.str(),
                    "My test 11");
    }

    {
        Database db;
        db.Add({2019, 12, 1}, "a");
        db.Add({2019, 12, 1}, "b");
        db.Add({2019, 12, 1}, "a");
        db.Add({2019, 12, 2}, "c");
        db.Add({2019, 12, 2}, "a");
        db.Add({2019, 12, 2}, "a");

        AssertEqual(2, DoRemove(db, R"(event == "a")"), "My test 12");
    }

    {
        Database db;
        db.Add({2019, 12, 1}, "a");
        db.Add({2019, 12, 1}, "aa");
        db.Add({2019, 12, 1}, "aaa");

        AssertEqual(2, DoRemove(db, R"(event >= "aa")"), "My test 13");

        ostringstream out;
        db.Print(out);
        AssertEqual("2019-12-01 a\n", out.str(), "My test 14");
    }

    {
        Database db;
        db.Add({2019, 12, 1}, "a");
        db.Add({2019, 12, 1}, "aa");
        db.Add({2019, 12, 1}, "aaa");
        db.Add({2019, 12, 2}, "b");
        db.Add({2019, 12, 2}, "a");

        AssertEqual(2, DoRemove(db, R"(event > "aa")"), "My test 15");

        ostringstream out;
        db.Print(out);
        AssertEqual("2019-12-01 a\n2019-12-01 aa\n2019-12-02 a\n", out.str(), "My test 16");
    }

    {
        Database db;
        db.Add({2019, 12, 1}, "a");
        db.Add({2019, 12, 1}, "aa");
        db.Add({2019, 12, 1}, "aaa");
        db.Add({2019, 12, 2}, "b");
        db.Add({2019, 12, 2}, "a");

        AssertEqual(2, DoRemove(db, R"(event < "aa")"), "My test 17");

        ostringstream out;
        db.Print(out);
        AssertEqual("2019-12-01 aa\n2019-12-01 aaa\n2019-12-02 b\n", out.str(), "My test 18");
    }

    {
        Database db;
        db.Add({2019, 12, 1}, "a");
        db.Add({2019, 12, 1}, "b");

        AssertEqual(1, DoRemove(db, R"(event != "b")"), "My test 19");

        db.Add({2019, 12, 1}, "c");

        AssertEqual(Entry({2019, 12, 1}, "c"), db.Last({2019, 12, 1}), " My test 20");

        db.Add({2019, 12, 1}, "b");
        AssertEqual(Entry({2019, 12, 1}, "c"), db.Last({2019, 12, 1}), " My test 21");

        ostringstream out;
        db.Print(out);
        AssertEqual("2019-12-01 b\n2019-12-01 c\n", out.str(), "My test 22");
    }


}

void TestDatabase() {
    istringstream empty_is("");
    auto empty_condition = ParseCondition(empty_is);
    auto empty_predicate = [empty_condition](const Date &date, const string &event) {
        return empty_condition->Evaluate(date, event);
    };

    // Add 2 - Del 1 - Add deleted again
    {
        Database db;
        Date d(2019, 1, 1);
        db.Add(d, "e1");
        db.Add(d, "e2");
        istringstream is(R"(event == "e1")");
        auto condition = ParseCondition(is);
        auto predicate = [condition](const Date &date, const string &event) {
            return condition->Evaluate(date, event);
        };
        AssertEqual(db.RemoveIf(predicate), 1, "Db Add2-Del-Add 1");
        db.Add(d, "e1");
        AssertEqual(db.FindIf(empty_predicate).size(), 2, "Db Add2-Del-Add 2");
    }

    // Add
    {
        Database db;
        Date d(2019, 1, 1);
        db.Add(d, "e1");
        db.Add(d, "e1");
        istringstream is("date == 2019-01-01");
        auto condition = ParseCondition(is);
        auto predicate = [condition](const Date &date, const string &event) {
            return condition->Evaluate(date, event);
        };
        AssertEqual(db.FindIf(predicate).size(), 1, "Db Add Duplicates 1");
    }

    // Last
    {
        Database db;
        Date d(2019, 1, 1);
        Date d1(2019, 1, 2);
        Date d2(2018, 12, 22);
        db.Add(d1, "e1");
        db.Add(d2, "e2");
        AssertEqual(db.Last(d), Entry({2018, 12, 22}, "e2"), "Db Last 1");
        Date d3(2018, 12, 24);
        db.Add(d3, "e3");
        AssertEqual(db.Last(d), Entry({2018, 12, 24}, "e3"), "Db Last 2");

        // Get last event for date before first event
        try {
            Date d4(2017, 2, 2);
            db.Last(d4);
            Assert(false, "Db Last 3");
        } catch (invalid_argument e) {
            // Pass
        }

        // Delete event and get last
        istringstream is("date == 2018-12-24");
        auto condition = ParseCondition(is);
        auto predicate = [condition](const Date &date, const string &event) {
            return condition->Evaluate(date, event);
        };
        db.RemoveIf(predicate);
        AssertEqual(db.Last(d), Entry({2018, 12, 22}, "e2"), "Db Last 4");

        AssertEqual(db.Last(d1), Entry({2019, 1, 2}, "e1"), "Db Last 5");
        db.Add(d2, "e4");
        AssertEqual(db.Last(d2), Entry({2018, 12, 22}, "e4"), "Db Last 6");
    }

    // Del
    {
        Database db;
        db.Add({2019, 1, 1}, "e1");
        db.Add({2019, 1, 1}, "e2");
        db.Add({2018, 1, 7}, "e3");
        db.Add({2018, 1, 7}, "e4");
        istringstream is("date == 2018-01-07");
        auto condition = ParseCondition(is);
        auto predicate = [condition](const Date &date, const string &event) {
            return condition->Evaluate(date, event);
        };
        AssertEqual(db.RemoveIf(predicate), 2, "Db Del 1");
    }
    {
        Database db;
        db.Add({2019, 1, 1}, "e1");
        db.Add({2019, 1, 1}, "e2");
        db.Add({2018, 1, 7}, "e3");
        db.Add({2018, 1, 7}, "e4");
        istringstream is("date >= 2018-01-07 AND date <= 2020-01-01");
        auto condition = ParseCondition(is);
        auto predicate = [condition](const Date &date, const string &event) {
            return condition->Evaluate(date, event);
        };
        AssertEqual(db.RemoveIf(predicate), 4, "Db Del 2");
    }
    {
        Database db;
        db.Add({2019, 1, 1}, "e1");
        db.Add({2019, 1, 1}, "e2");
        db.Add({2018, 1, 7}, "e3");
        db.Add({2018, 1, 7}, "e4");
        AssertEqual(db.RemoveIf(empty_predicate), 4, "Db Del 3");
    }
    {
        Database db;
        db.Add({2019, 1, 1}, "e1");
        db.Add({2019, 1, 1}, "e2");
        db.Add({2018, 1, 7}, "e3");
        db.Add({2018, 1, 7}, "e4");
        istringstream is(R"(event == "e1")");
        auto condition = ParseCondition(is);
        auto predicate = [condition](const Date &date, const string &event) {
            return condition->Evaluate(date, event);
        };
        AssertEqual(db.RemoveIf(predicate), 1, "Db Del 4");
    }

    {
        Database db;
        db.Add({2019, 1, 1}, "e1");
        db.Add({2019, 1, 1}, "e2");
        db.Add({2018, 1, 7}, "e3");
        db.Add({2018, 1, 7}, "e4");
        istringstream is(R"(event == "e1" OR date == 2019-01-01)");
        auto condition = ParseCondition(is);
        auto predicate = [condition](const Date &date, const string &event) {
            return condition->Evaluate(date, event);
        };
        AssertEqual(db.RemoveIf(predicate), 2, "Db Del 5");
    }

    // Find
    {
        Database db;
        db.Add({2019, 1, 1}, "e1");
        db.Add({2019, 1, 1}, "e2");
        db.Add({2018, 1, 7}, "e3");
        db.Add({2018, 1, 7}, "e4");
        istringstream is("date == 2018-01-07");
        auto condition = ParseCondition(is);
        auto predicate = [condition](const Date &date, const string &event) {
            return condition->Evaluate(date, event);
        };
        AssertEqual(db.FindIf(predicate).size(), 2, "Db Find 1");
    }
    {
        Database db;
        db.Add({2019, 1, 1}, "e1");
        db.Add({2019, 1, 1}, "e2");
        db.Add({2018, 1, 7}, "e3");
        db.Add({2018, 1, 7}, "e4");
        istringstream is("date >= 2018-01-07 AND date <= 2020-01-01");
        auto condition = ParseCondition(is);
        auto predicate = [condition](const Date &date, const string &event) {
            return condition->Evaluate(date, event);
        };
        AssertEqual(db.FindIf(predicate).size(), 4, "Db Find 2");
    }
    {
        Database db;
        db.Add({2019, 1, 1}, "e1");
        db.Add({2019, 1, 1}, "e2");
        db.Add({2018, 1, 7}, "e3");
        db.Add({2018, 1, 7}, "e4");
        AssertEqual(db.FindIf(empty_predicate).size(), 4, "Db Find 3");
    }
    {
        Database db;
        db.Add({2019, 1, 1}, "e1");
        db.Add({2019, 1, 1}, "e2");
        db.Add({2018, 1, 7}, "e3");
        db.Add({2018, 1, 7}, "e4");
        istringstream is(R"(event == "e1")");
        auto condition = ParseCondition(is);
        auto predicate = [condition](const Date &date, const string &event) {
            return condition->Evaluate(date, event);
        };
        AssertEqual(db.FindIf(predicate).size(), 1, "Db Find 4");
    }

    {
        Database db;
        db.Add({2019, 1, 1}, "e1");
        db.Add({2019, 1, 1}, "e2");
        db.Add({2018, 1, 7}, "e3");
        db.Add({2018, 1, 7}, "e4");
        istringstream is(R"(event == "e1" OR date == 2019-01-01)");
        auto condition = ParseCondition(is);
        auto predicate = [condition](const Date &date, const string &event) {
            return condition->Evaluate(date, event);
        };
        AssertEqual(db.FindIf(predicate).size(), 2, "Db Find 5");
    }

    // Add - Del - Add - Del
    {
        Database db;
        db.Add({2019, 1, 1}, "e1");
        db.Add({2019, 1, 1}, "e2");
        AssertEqual(db.RemoveIf(empty_predicate), 2, "Db Add-Del-Add-Del 1");

        db.Add({2019, 1, 1}, "e1");
        db.Add({2019, 1, 1}, "e2");
        AssertEqual(db.RemoveIf(empty_predicate), 2, "Db Add-Del-Add-Del 1");
    }

    {
        Database db;
        db.Add({2017, 1, 1}, "first");
        db.Add({2017, 1, 1}, "second");
        db.Add({2017, 1, 1}, "third");
        db.Add({2017, 1, 1}, "fourth");
        db.Add({2017, 1, 1}, "five");
        AssertEqual(2, DoRemove(db, R"(event == "second" OR event == "fourth")"), "Remove several");
        ostringstream os;
        db.Print(os);
        AssertEqual("2017-01-01 first\n2017-01-01 third\n2017-01-01 five\n", os.str(),
                    "Check print after remove several- 3");
    }
}

void TestDateComparisonNode() {
    {
        DateComparisonNode dcn(Comparison::Equal, {2017, 11, 18});
        Assert(dcn.Evaluate(Date{2017, 11, 18}, ""), "DateComparison 1");
        Assert(!dcn.Evaluate(Date{2017, 11, 19}, ""), "DateComparison 2");
    }
    {
        DateComparisonNode dcn(Comparison::NotEqual, {2017, 11, 18});
        Assert(dcn.Evaluate(Date{2017, 11, 19}, ""), "DateComparison 3");
        Assert(!dcn.Evaluate(Date{2017, 11, 18}, ""), "DateComparison 4");
    }
    {
        DateComparisonNode dcn(Comparison::Less, {2017, 11, 18});
        Assert(dcn.Evaluate(Date{2017, 11, 17}, ""), "DateComparison 5");
        Assert(!dcn.Evaluate(Date{2017, 11, 18}, ""), "DateComparison 6");
        Assert(!dcn.Evaluate(Date{2017, 11, 19}, ""), "DateComparison 7");
    }
    {
        DateComparisonNode dcn(Comparison::Greater, {2017, 11, 18});
        Assert(dcn.Evaluate(Date{2017, 11, 19}, ""), "DateComparison 8");
        Assert(!dcn.Evaluate(Date{2017, 11, 18}, ""), "DateComparison 9");
        Assert(!dcn.Evaluate(Date{2017, 11, 17}, ""), "DateComparison 10");
    }
    {
        DateComparisonNode dcn(Comparison::LessOrEqual, {2017, 11, 18});
        Assert(dcn.Evaluate(Date{2017, 11, 17}, ""), "DateComparison 11");
        Assert(dcn.Evaluate(Date{2017, 11, 18}, ""), "DateComparison 12");
        Assert(!dcn.Evaluate(Date{2017, 11, 19}, ""), "DateComparison 13");
    }
    {
        DateComparisonNode dcn(Comparison::GreaterOrEqual, {2017, 11, 18});
        Assert(dcn.Evaluate(Date{2017, 11, 19}, ""), "DateComparison 14");
        Assert(dcn.Evaluate(Date{2017, 11, 18}, ""), "DateComparison 15");
        Assert(!dcn.Evaluate(Date{2017, 11, 17}, ""), "DateComparison 16");
    }
}

void TestEventComparisonNode() {
    {
        EventComparisonNode ecn(Comparison::Equal, "abc");
        Assert(ecn.Evaluate(Date{0, 1, 1}, "abc"), "EventComparison 1");
        Assert(!ecn.Evaluate(Date{0, 1, 1}, "cbe"), "EventComparison 2");
    }
    {
        EventComparisonNode ecn(Comparison::NotEqual, "abc");
        Assert(ecn.Evaluate(Date{0, 1, 1}, "cde"), "EventComparison 3");
        Assert(!ecn.Evaluate(Date{0, 1, 1}, "abc"), "EventComparison 4");
    }
    {
        EventComparisonNode ecn(Comparison::Less, "abc");
        Assert(ecn.Evaluate(Date{0, 1, 1}, "abb"), "EventComparison 5");
        Assert(!ecn.Evaluate(Date{0, 1, 1}, "abc"), "EventComparison 6");
        Assert(!ecn.Evaluate(Date{0, 1, 1}, "abd"), "EventComparison 7");
    }
    {
        EventComparisonNode ecn(Comparison::Greater, "abc");
        Assert(ecn.Evaluate(Date{0, 1, 1}, "abd"), "EventComparison 8");
        Assert(!ecn.Evaluate(Date{0, 1, 1}, "abc"), "EventComparison 9");
        Assert(!ecn.Evaluate(Date{0, 1, 1}, "abb"), "EventComparison 10");
    }
    {
        EventComparisonNode ecn(Comparison::LessOrEqual, "abc");
        Assert(ecn.Evaluate(Date{0, 1, 1}, "abb"), "EventComparison 11");
        Assert(ecn.Evaluate(Date{0, 1, 1}, "abc"), "EventComparison 12");
        Assert(!ecn.Evaluate(Date{0, 1, 1}, "abd"), "EventComparison 13");
    }
    {
        EventComparisonNode ecn(Comparison::GreaterOrEqual, "abc");
        Assert(ecn.Evaluate(Date{0, 1, 1}, "abd"), "EventComparison 14");
        Assert(ecn.Evaluate(Date{0, 1, 1}, "abc"), "EventComparison 15");
        Assert(!ecn.Evaluate(Date{0, 1, 1}, "abb"), "EventComparison 16");
    }
}

void TestLogicalOperationNode() {
    {
        LogicalOperationNode lon(LogicalOperation::And, make_shared<EmptyNode>(), make_shared<EmptyNode>());
        Assert(lon.Evaluate(Date{0, 1, 1}, "abc"), "LogicalOperationNode 1");
    }
    {
        LogicalOperationNode lon(LogicalOperation::And, make_shared<AlwaysFalseNode>(), make_shared<EmptyNode>());
        Assert(!lon.Evaluate(Date{0, 1, 1}, "abc"), "LogicalOperationNode 2");
    }
    {
        LogicalOperationNode lon(LogicalOperation::And, make_shared<EmptyNode>(), make_shared<AlwaysFalseNode>());
        Assert(!lon.Evaluate(Date{0, 1, 1}, "abc"), "LogicalOperationNode 3");
    }
    {
        LogicalOperationNode lon(LogicalOperation::And, make_shared<AlwaysFalseNode>(), make_shared<AlwaysFalseNode>());
        Assert(!lon.Evaluate(Date{0, 1, 1}, "abc"), "LogicalOperationNode 4");
    }
    {
        LogicalOperationNode lon(LogicalOperation::Or, make_shared<EmptyNode>(), make_shared<EmptyNode>());
        Assert(lon.Evaluate(Date{0, 1, 1}, "abc"), "LogicalOperationNode 5");
    }
    {
        LogicalOperationNode lon(LogicalOperation::Or, make_shared<AlwaysFalseNode>(), make_shared<EmptyNode>());
        Assert(lon.Evaluate(Date{0, 1, 1}, "abc"), "LogicalOperationNode 6");
    }
    {
        LogicalOperationNode lon(LogicalOperation::Or, make_shared<EmptyNode>(), make_shared<AlwaysFalseNode>());
        Assert(lon.Evaluate(Date{0, 1, 1}, "abc"), "LogicalOperationNode 7");
    }
    {
        LogicalOperationNode lon(LogicalOperation::Or, make_shared<AlwaysFalseNode>(), make_shared<AlwaysFalseNode>());
        Assert(!lon.Evaluate(Date{0, 1, 1}, "abc"), "LogicalOperationNode 8");
    }
}

void TestEmptyNode() {
    {
        EmptyNode en;
        Assert(en.Evaluate(Date{0, 1, 1}, "abc"), "EmptyNode 1");
        Assert(en.Evaluate(Date{2017, 11, 18}, "def"), "EmptyNode 2");
        Assert(en.Evaluate(Date{9999, 12, 31}, "ghi"), "EmptyNode 3");
    }
}

void TestParseCondition() {
    {
        istringstream is("date != 2017-11-18");
        shared_ptr<Node> root = ParseCondition(is);
        Assert(root->Evaluate({2017, 1, 1}, ""), "Parse condition 1");
        Assert(!root->Evaluate({2017, 11, 18}, ""), "Parse condition 2");
    }
    {
        istringstream is(R"(event == "sport event")");
        shared_ptr<Node> root = ParseCondition(is);
        Assert(root->Evaluate({2017, 1, 1}, "sport event"), "Parse condition 3");
        Assert(!root->Evaluate({2017, 1, 1}, "holiday"), "Parse condition 4");
    }
    {
        istringstream is("date >= 2017-01-01 AND date < 2017-07-01");
        shared_ptr<Node> root = ParseCondition(is);
        Assert(root->Evaluate({2017, 1, 1}, ""), "Parse condition 5");
        Assert(root->Evaluate({2017, 3, 1}, ""), "Parse condition 6");
        Assert(root->Evaluate({2017, 6, 30}, ""), "Parse condition 7");
        Assert(!root->Evaluate({2017, 7, 1}, ""), "Parse condition 8");
        Assert(!root->Evaluate({2016, 12, 31}, ""), "Parse condition 9");
    }
    {
        istringstream is(R"(event != "sport event" AND event != "Wednesday")");
        shared_ptr<Node> root = ParseCondition(is);
        Assert(root->Evaluate({2017, 1, 1}, "holiday"), "Parse condition 10");
        Assert(!root->Evaluate({2017, 1, 1}, "sport event"), "Parse condition 11");
        Assert(!root->Evaluate({2017, 1, 1}, "Wednesday"), "Parse condition 12");
    }
    {
        istringstream is(R"(event == "holiday AND date == 2017-11-18")");
        shared_ptr<Node> root = ParseCondition(is);
        Assert(!root->Evaluate({2017, 11, 18}, "holiday"), "Parse condition 13");
        Assert(!root->Evaluate({2017, 11, 18}, "work day"), "Parse condition 14");
        Assert(root->Evaluate({1, 1, 1}, "holiday AND date == 2017-11-18"), "Parse condition 15");
    }
    {
        istringstream is(R"(((event == "holiday" AND date == 2017-01-01)))");
        shared_ptr<Node> root = ParseCondition(is);
        Assert(root->Evaluate({2017, 1, 1}, "holiday"), "Parse condition 16");
        Assert(!root->Evaluate({2017, 1, 2}, "holiday"), "Parse condition 17");
    }
    {
        istringstream is(R"(date > 2017-01-01 AND (event == "holiday" OR date < 2017-07-01))");
        shared_ptr<Node> root = ParseCondition(is);
        Assert(!root->Evaluate({2016, 1, 1}, "holiday"), "Parse condition 18");
        Assert(root->Evaluate({2017, 1, 2}, "holiday"), "Parse condition 19");
        Assert(root->Evaluate({2017, 1, 2}, "workday"), "Parse condition 20");
        Assert(!root->Evaluate({2018, 1, 2}, "workday"), "Parse condition 21");
    }
    {
        istringstream is(R"(date > 2017-01-01 AND event == "holiday" OR date < 2017-07-01)");
        shared_ptr<Node> root = ParseCondition(is);
        Assert(root->Evaluate({2016, 1, 1}, "event"), "Parse condition 22");
        Assert(root->Evaluate({2017, 1, 2}, "holiday"), "Parse condition 23");
        Assert(root->Evaluate({2017, 1, 2}, "workday"), "Parse condition 24");
        Assert(!root->Evaluate({2018, 1, 2}, "workday"), "Parse condition 25");
    }
    {
        istringstream is(R"(((date == 2017-01-01 AND event == "holiday")))");
        shared_ptr<Node> root = ParseCondition(is);
        Assert(root->Evaluate({2017, 1, 1}, "holiday"), "Parse condition 26");
        Assert(!root->Evaluate({2017, 1, 2}, "holiday"), "Parse condition 27");
    }
    {
        istringstream is(R"(((event == "2017-01-01" OR date > 2016-01-01)))");
        shared_ptr<Node> root = ParseCondition(is);
        Assert(root->Evaluate({1, 1, 1}, "2017-01-01"), "Parse condition 28");
        Assert(!root->Evaluate({2016, 1, 1}, "event"), "Parse condition 29");
        Assert(root->Evaluate({2016, 1, 2}, "event"), "Parse condition 30");
    }
}


void TestAll();

void TestParseEvent() {
    {
        istringstream is("event");
        AssertEqual(ParseEvent(is), "event", "Parse event without leading spaces");
    }
    {
        istringstream is("   sport event ");
        AssertEqual(ParseEvent(is), "sport event ", "Parse event with leading spaces");
    }
    {
        istringstream is("  first event  \n  second event");
        vector<string> events;
        events.push_back(ParseEvent(is));
        events.push_back(ParseEvent(is));
        AssertEqual(events, vector<string>{"first event  ", "second event"}, "Parse multiple events");
    }
}

void TestAll() {
    TestRunner tr;
    tr.RunTest(TestEmptyNode, "Test TestEmptyNode");
    tr.RunTest(TestDbAdd, "Test TestDbAdd");
    tr.RunTest(TestDbFind, "Test TestDbFind");
    tr.RunTest(TestDbLast, "Test TestDbLast");
    tr.RunTest(TestDbRemoveIf, "Test TestDbRemoveIf");
    tr.RunTest(TestInsertionOrder, "Test for the order of output");
    tr.RunTest(TestParseEvent, "TestParseEvent");
    tr.RunTest(TestParseCondition, "TestParseCondition");
}