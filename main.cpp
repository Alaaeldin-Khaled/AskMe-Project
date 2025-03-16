#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
using namespace std;

vector<string> splitString(string &str, char split = ',')
{
    vector<string> substrs;
    int pos;
    while ((pos = (int)str.find(split)) != -1)
    {
        string sub = str.substr(0, pos);
        str = str.substr(pos + 1);
        substrs.push_back(sub);
    }

    substrs.push_back(str);

    return substrs;
}

int toInt(string str)
{
    int i;
    stringstream ss;
    ss << str;
    ss >> i;

    return i;
}

struct User
{
    string username;
    string name;
    string email;
    int password;
    int id;
    bool allow_AQ;
    vector<int> questions_id;

    User() { id = 0; }

    User(string line)
    {
        vector<string> strs = splitString(line);

        this->id = toInt(strs[0]);
        this->username = strs[1];
        this->name = strs[2];
        this->email = strs[3];
        this->password = toInt(strs[4]);
        this->allow_AQ = toInt(strs[5]);
    }

    int login(string username, int password)
    {
        if (this->username == username && this->password == password)
        {
            return this->id;
        }
        return -1;
    }
};

struct UserManager
{
    int user_len;
    map<int, User> users;

    UserManager()
    {
        user_len = 1;
        load_users();
    }

    int user_exist(string username)
    {
        for (int i = 1; i <= user_len; ++i)
        {
            if (username == users[i].username)
                return users[i].id;
        }
        return -1;
    }

    bool user_exist(int id)
    {
        if (users[id].id != 0)
            return true;

        return false;
    }

    void createUser()
    {
        string username, name, email;
        int password, allow_AQ;

        User new_user;

        while (true)
        {
            cout << "Enter user name (no spaces): ";
            cin >> new_user.username;

            if (user_exist(new_user.username) != -1)
            {
                cout << "User Already Exist." << endl;
                continue;
            }

            break;
        }

        cout << "Enter password: ";
        cin >> new_user.password;
        cout << "Enter name: ";
        cin >> new_user.name;
        cout << "Enter Email: ";
        cin >> new_user.email;

        while (true)
        {
            cout << "Allow Anonymous Questions?: (0 or 1) ";
            cin >> new_user.allow_AQ;

            if (new_user.allow_AQ < 0 || new_user.allow_AQ > 1)
            {
                cout << "Invalid input" << endl;
                continue;
            }
            break;
        }

        new_user.id = user_len;
        users[user_len] = new_user;
        ++user_len;

        save_user(new_user);
    }

    int login()
    {
        string username;
        int password;

        cout << "Enter user name & password: ";
        cin >> username >> password;

        for (int i = 1; i <= user_len; ++i)
        {
            int user_id = users[i].login(username, password);
            if (user_id != -1)
                return user_id;
        }
        cout << "Wrong username or password" << endl;
        return -1;
    }

    void print_users()
    {
        for (int i = 1; i < user_len; ++i)
        {
            cout << "ID: " << users[i].id << "\t Name: " << users[i].name << endl;
        }
        cout << endl;
    }

    bool user_allow_AQ(int user_id)
    {
        return users[user_id].allow_AQ;
    }

    void save_user(User &u)
    {
        ofstream fout("users.txt", ios::app);

        if (fout.fail())
        {
            cout << "Can't open file." << endl;
            return;
        }


        fout << u.id << ",";
        fout << u.username << ",";
        fout << u.name << ",";
        fout << u.email << ",";
        fout << u.password << ",";
        fout << u.allow_AQ;
        fout << "\n";

        fout.close();
    }

    void load_users()
    {
        ifstream fin("users.txt");

        if (fin.fail())
        {
            cout << "Can't open file." << endl;
            return;
        }

        string line;

        stringstream iss;
        while (getline(fin, line))
        {
            User new_user(line);
            users[new_user.id] = new_user;
            user_len = new_user.id + 1;
            //cout << users[id].username;
        }
        //cout << user_len << endl;
        fin.close();
    }
};

struct Question
{
    string question_content;
    mutable string answer;
    int id, from_id, to_id;
    bool anonymous;
    int is_thread;

    Question() {}

    Question(string line)
    {
        vector<string> strs = splitString(line);

        this->id = toInt(strs[0]);
        this->from_id = toInt(strs[1]);
        this->to_id = toInt(strs[2]);
        this->anonymous = toInt(strs[3]);
        this->is_thread = toInt(strs[4]);
        this->question_content = strs[5];
        this->answer = strs[6];
    }

    Question(int id, int from_id, int to_id, string question_content, bool anonymous)
    {
        this->id = id;
        this->from_id = from_id;
        this->to_id = to_id;
        this->question_content = question_content;
        this->anonymous = anonymous;
        answer = "";
    }

    bool operator <(const Question& rhs) const
    {
        return id < rhs.id;
    }

    void print_question() const
    {
        cout << "Question ID (" << id << ") ";
        if (!anonymous)
            cout << "from user ID (" << from_id << ") ";
        cout << "to user ID (" << to_id << ")\t";
        cout << "Question: " << question_content << endl;
        if (answer != "")
            cout << "Answer: " << answer << endl;
    }

    void answer_question() const
    {
        string ans;
        cin.ignore();
        getline(cin, ans);
        answer = ans;
    }
};

struct QuestionManager
{
    int question_len;

    map<Question, vector<Question>> Questions;

    QuestionManager()
    {
        question_len = 1;
        load_questions();
    }

    ~QuestionManager()
    {
        save_questions();
    }

    // from_user_id = -1 if allow_AQ = true
    void ask_question(int from_user_id, int to_user_id, int question_id, string question_content, bool anonymous)
    {
        // if parent question
            Question q(question_len, from_user_id, to_user_id, question_content, anonymous);
            // cout << q.question_content << endl;
            if (question_id == -1)
            {
                Questions[q] = {};
                question_len++;
            }

            else
            {
                bool flag = true;
                for (auto &val : Questions)
                {
                    if ((val.first).id == question_id)
                    {
                        (val.second).push_back(q);
                        flag = false;
                        question_len++;
                        //cout << (val.second)[0].question_content << endl;
                    }
                }
                if (flag)
                    cout << "Parent Question not found." << endl;
            }
    }

    void print_questions()
    {
        for (auto &val : Questions)
        {
            (val.first).print_question();
            for (auto &v : (val.second))
            {
                cout << "Thread: ";
                v.print_question();
            }
//            for (int i = 0; i < (val.second).size(); ++i)
//            {
//                cout << "Thread: ";
//                (val.second)[i].print_question();
//            }

        }
    }

    bool question_exist(int id)
    {
        for (auto &val : Questions)
        {
            if (val.first.id == id)
                return true;
        }
        return false;
    }

    void answer_question(int id, int to_id)
    {
        for (auto &val : Questions)
        {
            if (val.first.id == id)
            {
                if (val.first.to_id != to_id)
                {
                    cout << "Question is not for you" << endl;
                    return;
                }
                cout << "Enter answer: ";
                (val.first).answer_question();
                return;
            }
            for (auto &v : val.second)
            {
                if (v.id == id)
                {
                    if (v.to_id != to_id)
                    {
                        cout << "Question is not for you" << endl;
                        return;
                    }
                    cout << "Enter answer: ";
                    v.answer_question();
                    return;
                }
            }
        }
        cout << "Question does not exist." << endl;
    }

    void print_questions_to_me(int user_id)
    {
        for (auto &val : Questions)
        {
            if (val.first.to_id == user_id)
                val.first.print_question();

            for (auto &v : val.second)
                if (v.to_id == user_id)
                {
                    cout << "Thread: ";
                    v.print_question();
                }
        }
    }

    void print_questions_from_me(int user_id)
    {
        for (auto &val : Questions)
        {
            if (val.first.from_id == user_id)
                val.first.print_question();

            for (auto &v : val.second)
                if (v.from_id == user_id)
                {
                    cout << "Thread: ";
                    v.print_question();
                }
        }
    }

    void delete_question(int question_id)
    {
        for (auto &[key, value] : Questions)
        {
            if (key.id == question_id)
            {
                auto it = Questions.find(key);
                Questions.erase(it);
                return;
            }

            for (int i = 0; i < value.size(); ++i)
            {
                if (value[i].id == question_id)
                {
                    value.erase(value.begin() + i);
                    return;
                }
            }
        }

        cout << "Question not found.." << endl;
    }

    void save_questions()
    {
        ofstream fout("questions.txt", ios::trunc);

        if (fout.fail())
        {
            cout << "Can't open file." << endl;
            return;
        }

        // Question data:
        // id, from_id, to_id, question_content, anonymous, answer

        for (auto &[key, value] : Questions)
        {
            int is_thread = -1;

            fout << key.id << ",";
            fout << key.from_id  << ",";
            fout << key.to_id  << ",";
            fout << key.anonymous << ",";
            fout << is_thread << ",";
            fout << key.question_content << ",";
            fout << key.answer;
            fout << "\n";

            is_thread = key.id;
            for (auto &v : value)
            {
                fout << v.id << ",";
                fout << v.from_id  << ",";
                fout << v.to_id  << ",";
                fout << v.anonymous << ",";
                fout << is_thread << ",";
                fout << v.question_content << ",";
                fout << v.answer;
                fout << "\n";
            }
        }

        fout.close();
    }

    void load_questions()
    {
        ifstream fin("questions.txt");

        if (fin.fail())
        {
            cout << "Can't open file." << endl;
            return;
        }

        string line;

        stringstream iss;
        while (getline(fin, line))
        {

            // Question data:
            // id, from_id, to_id, question_content, anonymous, answer

            Question new_question(line);

            if (new_question.is_thread == -1)
            {
                Questions[new_question] = {};
            }
            else
            {
                for (auto &[key, val] : Questions)
                {
                    if (key.id == new_question.is_thread)
                    {
                        val.push_back(new_question);
                        break;
                    }
                }
            }

            question_len++;
        }

        fin.close();
    }
};

struct AskMe
{
    UserManager usermanager;
    QuestionManager questionmanager;

    int user_id; // logged in user id

    AskMe() {}

    void signUp()
    {
        usermanager.createUser();
    }

    void login()
    {
        user_id = usermanager.login();
    }

    void logout()
    {
    }

    void print_questions_to_me()
    {
        questionmanager.print_questions_to_me(user_id);
    }

    void print_questions_from_me()
    {
        questionmanager.print_questions_from_me(user_id);
    }

    void answer_question()
    {
        int question_id;

        cout << "Enter Question Id or -1 to cancel: ";
        cin >> question_id;
        if (question_id == -1)
            return;

        questionmanager.answer_question(question_id, user_id);
    }

    void delete_question()
    {
        int question_id;
        cout << "Enter Question ID or -1 to cancel: ";
        cin >> question_id;
        if (question_id == -1)
            return;

        questionmanager.delete_question(question_id);
    }

    void ask_question()
    {
        int to_user_id;
        bool allow_AQ;
        int question_id;
        string question_text;

        while (true)
        {
            cout << "Enter User ID or -1 to cancel: ";

            cin >> to_user_id;
            if (to_user_id == -1)
                return;

            if (!usermanager.user_exist(to_user_id))
            {
                cout << "User does not exist" << endl;
                continue;
            }
            break;
        }

        allow_AQ = usermanager.user_allow_AQ(to_user_id);

        if (!allow_AQ)
        {
            cout << "Note: Anonymous questions are not allowed for this user" << endl;
        }

        while (true)
        {
            cout << "For thread question: Enter Question id or -1 for new question: ";
            cin >> question_id;
            if (question_id == -1)
                break;

            if (questionmanager.question_exist(question_id))
                break;
            cout << "Question does not exist." << endl;
        }


        cout << "Enter Question content: ";

        cin.ignore();
        getline(cin, question_text);

        // assume thread question

        // assume -1

        int from_user_id = user_id;

        questionmanager.ask_question(from_user_id, to_user_id, question_id, question_text, allow_AQ);
    }

    void list_system_users()
    {
        usermanager.print_users();
    }

    void feed()
    {
        questionmanager.print_questions();
    }

    int menu1()
    {
        int choice;
        cout << "\nMenu:\n";
        cout << "\t1: Login\n";
        cout << "\t2: Sign Up\n\n";
        cout << "Enter number in range 1 - 2: ";
        cin >> choice;

        if (choice < 1 || choice > 2)
        {
            cout << "Invalid Input..\n";
            choice = menu1();
        }
        return choice;
    }

    int menu2()
    {
        usermanager.load_users();
        questionmanager.load_questions();

        int choice;
        cout << "\nMenu:\n";
        cout << "\t1: Print Questions To Me\n";
        cout << "\t2: Print Questions From Me\n";
        cout << "\t3: Answer Question\n";
        cout << "\t4: Delete Question\n";
        cout << "\t5: Ask Question\n";
        cout << "\t6: List System Users\n";
        cout << "\t7: Feed\n";
        cout << "\t8: Logout\n";

        cout << "\nEnter number in range 1 - 8: ";
        cin >> choice;

        if (choice < 1 || choice > 8)
        {
            cout << "Error: Invalid number... Try again\n";
            choice = menu2();
        }
        return choice;

        cout << endl;
    }

    void AskMe_system()
    {
        int choice;

        while (true)
        {
            choice = menu1();

            if (choice == 1)
            {
                login();

                if (user_id != -1)
                {
                    break;
                }
            }
            else
            {
                signUp();
            }
        }

        while (true)
        {
            choice = menu2();

            if (choice == 1)
                print_questions_to_me();
            else if (choice == 2)
                print_questions_from_me();
            else if (choice == 3)
                answer_question();
            else if (choice == 4)
                delete_question();
            else if (choice == 5)
                ask_question();
            else if (choice == 6)
                list_system_users();
            else if (choice == 7)
                feed();
            else if (choice == 8)
            {
                logout();
                break;
            }
        }
    }
};

int main()
{
    AskMe my_askme;
    my_askme.AskMe_system();
    return 0;
}
