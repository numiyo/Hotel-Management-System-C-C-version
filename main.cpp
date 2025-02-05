/*
#-------------------------------------_________-----__________________________-----------------------          ——————————————————
#                                          -——————                                     ——————————————————
#                                ————————                                                          ————————————————       ——————————
#__________——————————————————————             ______--------------————————————————————————————-----_______——————————————___
#
#         ┌─┐       ┌─┐
#      ┌──┘ ┴───────┘ ┴──┐
#      │                 │                          项目:  基于c++的本地酒店管理系统
#      │       ───       │                          author:  numiyo
#      │  ─┬┘       └┬─  │                          语言环境:  C++11及以上
#      │                 │                          版本控制:  git-github
#      │       ─┴─       │                          编辑器:  CLion-SublimeText
#      │                 │                          数据库:  本地txt文本
#      └───┐         ┌───┘                          更多详见:  https://github.com/numiyo/Hotel-Management-System-C-Cpp-version
#          │         │
#          │         │                              ┌─┐    ┌─┐
#          │         │                            ┌─┘ ┴────┘ ┴──┐
#          │         └──────────────┐             │             │
#          │                        │             │ ─┬     ─┬   │
#          │                        ├─┐           │    ─┴─      │
#          │                        ┌─┘           └───┐     ┌───┘
#          │                        │                 │     └─────────────┬─┐
#          └─┐  ┐  ┌───────┬──┐  ┌──┘                 │                   ├─┘
#            │ ─┤ ─┤       │ ─┤ ─┤                    │     ┌──────┬──┐  ┌┘
#            └──┴──┘       └──┴──┘                    └──┴──┘      └──┴──┘
———————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
*/

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iomanip>
#include <ctime>
#include <algorithm>
using namespace std;

//现在登录的账号
string Now_User;

//如果要实现多机器同时操作共享文件，可以把以下布尔数拉进用户结构体，再进行解析以及读取修改操作实现单一用户对应状态，本项目只是为了实现单机本地非联网构建的学习项目
//登录状态
bool IsUserLogin = false;

//检测程序文件是否正常读取
bool IsDataRunning = true;

//交易进行状态
bool IsTradeCompleted = false;

//直接使用txt文件作为用户注册信息以及房间信息的存储库
string UserData = IsDataRunning ? "data/user.txt" : "../data/user.txt";
string RoomData = IsDataRunning ? "data/room.txt" : "../data/room.txt";
string RequestData = IsDataRunning ? "data/request.txt" : "../data/request.txt";

//用户结构体
struct User {
    string username;//用户名
    string password;//密码
    string role = "Ordinary member"; //身份(普通会员"Ordinary member",Vip会员"VIP member",管理员"Administrator")
    double balance = 0.0;//余额默认为0
};

//客房结构体
struct Room {
    int roomNumber;//房间号
    string roomType;//房间类型（单人间"Single"、双人间"Double"、套房"Suite"）
    double price;//房间价格(每天)
    int reservedCount;//预订房间的人数（该房间的预定订单数）
    string bookedBy;//预订房间的用户
    string startDate;//预定开始时间
    string endDate;//预定结束时间
    vector<pair<string, pair<string, string>>> later_bookedBy;//后续预订信息（用户名、开始时间、结束时间）
};

//请求结构体
struct Request {
    int requestNumber;//订单号(按请求时间顺序排)
    string username;//操作者(要充值的人)
    double Top_up;//充值金额
    string requestDate;//请求时间
};

//时间
//获取当前时间
string GetCurrentDate () {
    time_t now = time(0);//获取当前时间
    tm* ltm = localtime(&now);//转换为本地时间
    char buffer[11];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d", ltm);//格式化为YYYY-MM-DD
    return string(buffer);
}

// 时间处理（天）
int DaysBetweenDates (const string& start, const string& end) {
    tm startDate = {};
    tm endDate = {};
    sscanf(start.c_str(), "%4d-%2d-%2d", &startDate.tm_year, &startDate.tm_mon, &startDate.tm_mday);
    sscanf(end.c_str(), "%4d-%2d-%2d", &endDate.tm_year, &endDate.tm_mon, &endDate.tm_mday);
    startDate.tm_year -= 1900;
    endDate.tm_year -= 1900;
    time_t start_time = mktime(&startDate);
    time_t end_time = mktime(&endDate);
    double seconds = difftime(end_time, start_time);
    return static_cast<int>(seconds / (60 * 60 * 24));
}

//用户
//用户数据读取
vector<User> Load_Users () {
    vector<User> users;
    ifstream file(UserData);
    string line;
    while (getline(file, line)) {
        User user;
        size_t pos = 0;
        string token;
        vector<string> tokens;
        while (!line.empty()) {
        	pos = line.find('|');
        	if (pos != string::npos){
            	//读取最前面到'|'所有的字符并赋值
            	token = line.substr(0, pos);
            	tokens.push_back(token);
            	//消除掉最前面到'|'的这一大串，继续循环取值；
            	line.erase(0, pos + 1);
        	} else {
                //读入最后一个数据
				token = line;
				tokens.push_back(token);
				line.erase();
			}
        }
        //把值赋值给机构体user的结构体的这个用户名和密码
        user.username = tokens[0];
        user.password = tokens[1];
        user.role = tokens[2];
        //使用stod函数把字符串转换成double类型
        user.balance = stod(tokens[3]);
        //放到容器里面
        users.push_back(user);
    }
    file.close();
    return users;
}

//用户数据写入
void Save_Users (const vector<User>& users) {
    ofstream file(UserData);
    for (auto & user : users) {
        file << user.username << "|" << user.password << "|" << user.role << "|" << user.balance << endl;
    }
    file.close();
}

//用户身份读取
string View_Role () {
    vector<User> users = Load_Users();
    for (auto & user : users) {
        if (user.username == Now_User) {
            return user.role;
        }
    }
    return "None";
}

//用户余额读取
void View_Balance () {
    vector<User> users = Load_Users();
    for (auto & user : users) {
        if (user.username == Now_User) {
            cout << "Your current balance is: $" << user.balance << endl;
            return;
        }
    }
}

//用户余额读取(返回数值参与计算)
double Get_Balance () {
    vector<User> users = Load_Users();
    for (const auto& user : users) {
        if (user.username == Now_User) {
            return user.balance;
        }
    }
    return 0.0;
}

//用户折扣查询
double View_Discount (string UserRole) {
    if (UserRole == "Administrator") {
        return 0.0;
    } else if (UserRole == "VIP member") {
        return 0.8;
    } else if (UserRole == "Ordinary member") {
        return 1.0;
    }
    return 1.0;
}

//用户余额充值
void Recharge_Balance () {
    cout << "Do you want to Top up your balance (y/n)? : ";
    char choose;
    cin >> choose;
    if (choose != 'y' && choose != 'Y') {
        return;
    }
    vector<Request> requests;
    ifstream file(RequestData);
    string line;
    while (getline(file, line)) {
        Request request;
        size_t pos = 0;
        string token;
        vector<string> tokens;
        while (!line.empty()) {
            pos = line.find('|');
            if (pos != string::npos) {
                token = line.substr(0, pos);
                tokens.push_back(token);
                line.erase(0, pos + 1);
            } else {
                token = line;
                tokens.push_back(token);
                line.erase();
            }
            if (tokens.size() != 4) continue;
            request.requestNumber = stoi(tokens[0]);
            request.username = tokens[1];
            request.Top_up = stod(tokens[2]);
            request.requestDate = tokens[3];
            requests.push_back(request);
        }
    }
    file.close();
    string money;
    cout << "How much do you want to top up: $";
    cin >> money;
    try {
        int Top_up = stoi(money);
    }
    catch (invalid_argument &) {
        cout << "Invalid_argument." << endl;
        return;
    }
    catch (out_of_range &) {
        cout << "Out of range." << endl;
        return;
    }
    catch (...) {
        cout << "Something else error." << endl;
        return;
    }
    double Top_up = stod(money);
    //创建充值请求
    Request newRequest;
    newRequest.requestNumber = requests.empty() ? 1 : requests.back().requestNumber + 1;
    newRequest.username = Now_User;
    newRequest.Top_up = Top_up;
    newRequest.requestDate = GetCurrentDate();
    //添加到请求列表
    requests.push_back(newRequest);
    //写入文件
    ofstream outFile(RequestData);
    for (const auto &request: requests) {
        outFile << request.requestNumber << "|" << request.username << "|" << request.Top_up << "|"
                << request.requestDate << endl;
    }
    outFile.close();
    IsTradeCompleted = true;
}

//房间
//按房间号排序
bool compareRoomsByNumber (const Room& a, const Room& b) {
    return a.roomNumber < b.roomNumber;
}

//按房间号排序
void sortRoomsByNumber (vector<Room>& rooms) {
    sort(rooms.begin(), rooms.end(), compareRoomsByNumber);
}

//房间后续订单排序规则
bool CompareBookings (const pair<string, pair<string, string>>& a, const pair<string, pair<string, string>>& b) {
    //比较预订开始时间
    return DaysBetweenDates(a.second.first, "2077-01-01") > DaysBetweenDates(b.second.first, "2077-01-01");
}

//对房间的后续预订信息进行排序
void SortLaterBookings (Room& room) {
    //对later_bookedBy列表进行排序
    sort(room.later_bookedBy.begin(), room.later_bookedBy.end(), CompareBookings);
}

//房间数据再重载
void AdjustBookings (Room& room) {
    //检查后续预订列表是否为空或第一个预订的开始日期是否早于当前预订的开始日期
    if (!room.later_bookedBy.empty() && room.later_bookedBy.front().second.first < room.startDate) {
        //保存需要移动到当前预订位置的后续预订信息
        auto nextBooking = room.later_bookedBy.front();
        //将原来的当前预订信息添加到后续预订列表的末尾
        room.later_bookedBy.push_back({room.bookedBy, {room.startDate, room.endDate}});
        //将后续预订的第一个元素移动到当前预订位置
        room.bookedBy = nextBooking.first;
        room.startDate = nextBooking.second.first;
        room.endDate = nextBooking.second.second;
        //从后续预订列表中移除已移动的元素
        room.later_bookedBy.erase(room.later_bookedBy.begin());
    }
}

//房间数据读取
vector<Room> Load_Rooms () {
    vector<Room> rooms;
    ifstream file(RoomData);
    string line;
    while (getline(file, line)) {
        Room room;
        size_t pos = 0;
        string token;
        vector<string> tokens;
        while (!line.empty()) {
            pos = line.find('|');
            if (pos != string::npos) {
                token = line.substr(0, pos);
                tokens.push_back(token);
                line.erase(0, pos + 1);
            } else {
                token = line;
                tokens.push_back(token);
                line.erase();
            }
        }
        room.roomNumber = stoi(tokens[0]);
        room.roomType = tokens[1];
        room.price = stod(tokens[2]);
        room.reservedCount = stoi(tokens[3]);
        room.bookedBy = tokens[4];
        room.startDate = tokens[5];
        room.endDate = tokens[6];
        vector<pair<string, pair<string, string>>> laterBooked;
        //解决后续预定信息
        for (size_t i = 7; i < tokens.size(); i += 3) {
            laterBooked.push_back({tokens[i], {tokens[i + 1], tokens[i + 2]}});
        }
        room.later_bookedBy = laterBooked;
        SortLaterBookings (room);//排序后续预订信息
        AdjustBookings(room);//排序当前所有订房信息
        SortLaterBookings (room);//再排序后续预订信息
        rooms.push_back(room);
        sortRoomsByNumber(rooms);//排序房间号
    }
    file.close();
    return rooms;
}

//房间状态读取
void View_Rooms (const vector<Room>& rooms) {
    cout << setw(7) << left << "Room"
         << setw(9) << left << "Type"
         << setw(8) << left << "Price"
         << setw(12) << left << "Reserved"
         << setw(12) << left << "Booked-By"
         << setw(13) << left << "Start-Date"
         << setw(12) << left << "End-Date" << endl;
    for (const auto& room : rooms) {
        cout << setw(7) << left << room.roomNumber
             << setw(9) << left << room.roomType
             << "$" << setw(10) << left << room.price
             << setw(11) << left << room.reservedCount
             << setw(10) << left << room.bookedBy
             << setw(12) << left << room.startDate
             << setw(12) << left << room.endDate << endl;
    }
}

//用户房间获取
void View_Booked_Rooms () {
    vector<Room> rooms = Load_Rooms();
    bool hasBookings = false;
    cout << setw(7) << left << "Room"
         << setw(9) << left << "Type"
         << setw(8) << left << "Price"
         << setw(12) << left << "Booked-By"
         << setw(13) << left << "Start-Date"
         << setw(12) << left << "End-Date" << endl;
    for (const auto& room : rooms) {
        //检查当前预订是否属于当前用户
        if (room.bookedBy == Now_User) {
            hasBookings = true;
            cout << setw(7) << left << room.roomNumber
                 << setw(9) << left << room.roomType
                 << "$" << setw(9) << left << room.price
                 << setw(10) << left << room.bookedBy
                 << setw(12) << left << room.startDate
                 << setw(12) << left << room.endDate << endl;
        }
        //遍历后续预订，查找属于当前用户的所有预订
        for (const auto& later : room.later_bookedBy) {
            if (later.first == Now_User) {
                hasBookings = true;
                cout << setw(7) << left << room.roomNumber
                     << setw(9) << left << room.roomType
                     << "$" << setw(9) << left << room.price
                     << setw(10) << left << later.first
                     << setw(12) << left << later.second.first
                     << setw(12) << left << later.second.second << endl;
            }
        }
    }
    if (!hasBookings) {
        cout << "You have no bookings." << endl;
    }
}

//房间数据保存
void Save_Rooms (const vector<Room>& rooms) {
    ofstream file(RoomData);
    for (const auto& room : rooms) {
        file << room.roomNumber << "|" << room.roomType << "|" << room.price << "|" << room.reservedCount << "|"
             << room.bookedBy << "|" << room.startDate << "|" << room.endDate;
        for (const auto& later : room.later_bookedBy) {
            file << "|" << later.first << "|" << later.second.first << "|" << later.second.second;
        }
        file << endl;
    }
    file.close();
}

//订房功能
void Book_Room (string number) {
    vector<Room> rooms = Load_Rooms();
    try {
        int choice = stoi(number);
    }
    catch (invalid_argument&){
        cout << "Invalid_argument." << endl;
        return;
    }
    catch (out_of_range&){
        cout << "Out of range." << endl;
        return;
    }
    catch (...) {
        cout << "Something else error." << endl;
        return;
    }
    int choice = stoi(number);
    Room selectedRoom;
    bool found = false;
    // 查找房间
    for (auto &room : rooms) {
        if (room.roomNumber == choice) {
            selectedRoom = room;
            found = true;
            break;
        }
    }
    if (!found) {
        cout << "Room does not exist." << endl;
        return;
    }
    //显示房间已预订的日期
    if (selectedRoom.bookedBy != "None") {
        cout << "Room " << selectedRoom.roomNumber << " is already booked on the following dates:" << endl;
        cout << "From " << selectedRoom.startDate << " to " << selectedRoom.endDate << endl;
        for (const auto& later : selectedRoom.later_bookedBy) {
            cout << "From " << later.second.first << " to " << later.second.second << endl;
        }
    }
    while (true) {
        char choose;
        cout << "Do you want to book this room (y/n)? : ";
        cin >> choose;
        if (choose == 'N' || choose == 'n') {
            return;
        } else if (choose == 'Y' || choose == 'y') {
            break;
        }
    }
    //获取用户输入的日期
    string startDate, endDate;
    cout << "Enter check-in date (YYYY-MM-DD): ";
    cin >> startDate;
    cout << "Enter check-out date (YYYY-MM-DD): ";
    cin >> endDate;
    //检查日期是否有效
    if (startDate > endDate || startDate < GetCurrentDate() || startDate.length() != 10 || endDate.length() != 10) {
        cout << "Invalid date range or check-in date is in the past." << endl;
        return;
    }
    //检查时间冲突
    if (selectedRoom.bookedBy != "None" && startDate <= selectedRoom.endDate && startDate >= selectedRoom.startDate) {
        cout << "Room is already booked during this period." << endl;
        return;
    }
    for (const auto& later : selectedRoom.later_bookedBy) {
        if (startDate <= later.second.second && endDate >= later.second.first) {
            cout << "Room is already booked during this period." << endl;
            return;
        }
    }
    //计算天数和费用
    int days = DaysBetweenDates(startDate, endDate) + 1;
    if (days <= 0) {
        cout << "Invalid date range." << endl;
        return;
    }
    double totalCost = selectedRoom.price * days * View_Discount(View_Role());
    if (totalCost > Get_Balance()) {
        cout << "Insufficient balance." << endl;
        cout << "Required: $" << totalCost << endl;
        return;
    }
    //确认预订
    cout << "You are about to book room " << selectedRoom.roomNumber << " for " << days << (days > 1 ? " days." : " day.") << endl;
    cout << "Total cost: $" << totalCost << endl;
    char confirm;
    cout << "Confirm booking (y/n)? : ";
    cin >> confirm;
    if (confirm == 'y' || confirm == 'Y') {
        //更新用户余额
        double newBalance = Get_Balance() - totalCost;
        vector<User> users = Load_Users();
        for (auto &user : users) {
            if (user.username == Now_User) {
                user.balance = newBalance;
                break;
            }
        }
        Save_Users(users);
        //更新房间预订信息
        if (selectedRoom.bookedBy == "None") {
            selectedRoom.bookedBy = Now_User;
            selectedRoom.startDate = startDate;
            selectedRoom.endDate = endDate;
            selectedRoom.reservedCount++;
        } else {
            selectedRoom.later_bookedBy.push_back({Now_User, {startDate, endDate}});
            selectedRoom.reservedCount++;
        }
        //更新所有房间信息
        for (auto& room : rooms) {
            if (room.roomNumber == selectedRoom.roomNumber) {
                room = selectedRoom;
                break;
            }
        }
        Save_Rooms(rooms);//保存更新后的房间信息
        IsTradeCompleted = true;
    }
}

//退房功能迭代器
bool HasLaterBookingByCurrentUser (const Room& room) {
    return find_if(room.later_bookedBy.begin(), room.later_bookedBy.end(), [&](const auto& entry) {
        return entry.first == Now_User;
    }) != room.later_bookedBy.end();
}

//退房功能
void Cancel_Booking (string number) {
    vector<Room> rooms = Load_Rooms();
    try {
        int choice = stoi(number);
    }
    catch (invalid_argument &) {
        cout << "Invalid_argument." << endl;
        return;
    }
    catch (out_of_range &) {
        cout << "Out of range." << endl;
        return;
    }
    catch (...) {
        cout << "Something else error." << endl;
        return;
    }
    int choice = stoi(number);
    Room selectedRoom;
    bool found = false;
    double refundAmount;
    double penalty;
    double totalled_refund = 0.0;
    double totalled_penalty = 0.0;
    // 查找房间
    for (auto &room: rooms) {
        if (room.roomNumber == choice) {
            selectedRoom = room;
            found = true;
            break;
        }
    }
    if (!found) {
        cout << "Room does not exist." << endl;
        return;
    }
    bool okay = false;
    //检查房间是否由当前用户预订 and 计算退款金额（实现的是全部该用户关于该房间的订单都退）
    while (true) {
        if (!HasLaterBookingByCurrentUser(selectedRoom)) break;
        for (auto it = selectedRoom.later_bookedBy.begin(); it != selectedRoom.later_bookedBy.end() + 1; it++) {
            if (it->first == Now_User) {
                //计算退款金额
                okay = true;
                if (it->second.first > GetCurrentDate()) {
                    int days = DaysBetweenDates(it->second.first, it->second.second) + 1;
                    refundAmount = selectedRoom.price * days * View_Discount(View_Role());
                    penalty = refundAmount * 0.1; //违约金为10%
                    refundAmount -= penalty;
                } else {
                    int days = DaysBetweenDates(GetCurrentDate(), it->second.second) + 1;
                    refundAmount = selectedRoom.price * days * View_Discount(View_Role());
                    penalty = refundAmount * 0.1; //违约金为10%
                    refundAmount -= penalty;
                }
                totalled_refund += refundAmount;
                totalled_penalty += penalty;
                //更新用户余额
                double newBalance = Get_Balance() + refundAmount;
                vector<User> users = Load_Users();
                for (auto &user: users) {
                    if (user.username == Now_User) {
                        user.balance = newBalance;
                        break;
                    }
                }
                Save_Users(users);
                //更新房间状态
                selectedRoom.later_bookedBy.erase(it);
                selectedRoom.reservedCount--;
            }
        }
    }
    if (selectedRoom.bookedBy == Now_User) {
        okay = true;
        //计算退款金额
        if (selectedRoom.startDate > GetCurrentDate()) {
            int days = DaysBetweenDates(selectedRoom.startDate, selectedRoom.endDate) + 1;
            refundAmount = selectedRoom.price * days * View_Discount(View_Role());
            penalty = refundAmount * 0.1; //违约金为10%
            refundAmount -= penalty;
        } else {
            int days = DaysBetweenDates(GetCurrentDate(), selectedRoom.endDate) + 1;
            refundAmount = selectedRoom.price * days * View_Discount(View_Role());
            penalty = refundAmount * 0.1; //违约金为10%
            refundAmount -= penalty;
        }
        totalled_refund += refundAmount;
        totalled_penalty += penalty;
        //更新用户余额
        double newBalance = Get_Balance() + refundAmount;
        vector<User> users = Load_Users();
        for (auto& user : users) {
            if (user.username == Now_User) {
                user.balance = newBalance;
                break;
            }
        }
        Save_Users(users);
        // 更新房间状态
        selectedRoom.bookedBy = "None";
        selectedRoom.startDate = GetCurrentDate();
        selectedRoom.endDate = GetCurrentDate();
        selectedRoom.reservedCount--;
        if (!selectedRoom.later_bookedBy.empty()) {
            selectedRoom.bookedBy = selectedRoom.later_bookedBy.front().first;
            selectedRoom.startDate = selectedRoom.later_bookedBy.front().second.first;
            selectedRoom.endDate = selectedRoom.later_bookedBy.front().second.second;
            selectedRoom.later_bookedBy.erase(selectedRoom.later_bookedBy.begin());
        }
    }
    if (!okay) {
        cout << "You do not have a booking for this room." << endl;
        return;
    }
    //更新所有房间信息
    for (auto& room : rooms) {
        if (room.roomNumber == selectedRoom.roomNumber) {
            room = selectedRoom;
            break;
        }
    }
    Save_Rooms(rooms); //保存更新后的房间信息
    IsTradeCompleted = true;
    cout << "Cancellation successful. \nRefund amount: $" << totalled_refund << " (Penalty: $" << totalled_penalty << ")" << endl;
}

//查房功能
void Find_Rooms (int order) {
    if (order == 1) {
        vector<Room> rooms = Load_Rooms();
        string startDate, endDate;
        //懒得一直get了-_-
        string currentDate = GetCurrentDate();
        //获取用户输入的起始日期和结束日期
        cout << "Enter the check-in date (YYYY-MM-DD): ";
        cin >> startDate;
        cout << "Enter the check-out date (YYYY-MM-DD): ";
        cin >> endDate;
        //检查日期格式和有效性
        if (startDate.length() != 10 || endDate.length() != 10 || startDate < currentDate || startDate > endDate) {
            cout << "Invalid date range.\nThe start date must be after the current date and the end date must be after the start date." << endl;
            return;
        }
        bool hasAvailableRooms = false;
        cout << setw(7) << left << "Room"
             << setw(9) << left << "Type"
             << setw(8) << left << "Price"
             << setw(12) << left << "Reserved"
             << setw(12) << left << "Booked-By"
             << setw(13) << left << "Start-Date"
             << setw(12) << left << "End-Date" << endl;
        for (const auto &room: rooms) {
            bool isAvailable = true;
            //检查当前预订是否冲突
            if (room.bookedBy != "None") {
                if (!(startDate > room.endDate || endDate < room.startDate)) {
                    isAvailable = false;
                }
            }
            //检查后续预订是否冲突
            for (const auto &later: room.later_bookedBy) {
                if (!(startDate > later.second.second || endDate < later.second.first)) {
                    isAvailable = false;
                    break;
                }
            }
            if (isAvailable) {
                hasAvailableRooms = true;
                cout << setw(7) << left << room.roomNumber
                     << setw(9) << left << room.roomType
                     << "$" << setw(10) << left << room.price
                     << setw(11) << left << room.reservedCount
                     << setw(10) << left << room.bookedBy
                     << setw(12) << left << room.startDate
                     << setw(12) << left << room.endDate << endl;
            }
        }
        if (!hasAvailableRooms) {
            cout << "No available rooms for the specified date range." << endl;
        }
    } else if (order == 2) {
        vector<Room> rooms = Load_Rooms();
        string roomType;
        string currentDate = GetCurrentDate();
        bool hasAvailableRooms = false;
        //输入要查询的房间类型
        cout << "Enter the Type of rooms（Single, Double, Suite, etc）: ";
        cin >> roomType;
        cout << setw(7) << left << "Room"
             << setw(9) << left << "Type"
             << setw(8) << left << "Price"
             << setw(12) << left << "Reserved"
             << setw(12) << left << "Booked-By"
             << setw(13) << left << "Start-Date"
             << setw(12) << left << "End-Date" << endl;
        for (const auto &room: rooms) {
            if (room.roomType == roomType) {
                hasAvailableRooms = true;
                //输出符合条件的房间信息
                cout << setw(7) << left << room.roomNumber
                     << setw(9) << left << room.roomType
                     << "$" << setw(10) << left << room.price
                     << setw(11) << left << room.reservedCount
                     << setw(10) << left << room.bookedBy
                     << setw(12) << left << room.startDate
                     << setw(12) << left << room.endDate << endl;
            }
        }
        //如果没有找到符合条件的房间，给出提示
        if (!hasAvailableRooms) {
            cout << "No available rooms for the specified Room Type." << endl;
        }
    } else if (order == 3) {
        vector<Room> rooms = Load_Rooms();
        string minPrice_str, maxPrice_str;
        string currentDate = GetCurrentDate();
        bool hasAvailableRooms = false;
        //提示用户输入最低价格和最高价格
        cout << "Enter the minimum price: $";
        cin >> minPrice_str;
        cout << "Enter the maximum price: $";
        cin >> maxPrice_str;
        try {
            double minPrice = stod(minPrice_str);
            double maxPrice = stod(maxPrice_str);
        }
        catch (invalid_argument &) {
            cout << "Invalid_argument." << endl;
            return;
        }
        catch (out_of_range &) {
            cout << "Out of range." << endl;
            return;
        }
        catch (...) {
            cout << "Something else error." << endl;
            return;
        }
        double minPrice = stod(minPrice_str);
        double maxPrice = stod(maxPrice_str);
        //检查价格区间的有效性
        if (minPrice > maxPrice) {
            cout << "Invalid price range.\nThe minimum price cannot be higher than the maximum price." << endl;
            return;
        }
        cout << setw(7) << left << "Room"
             << setw(9) << left << "Type"
             << setw(8) << left << "Price"
             << setw(12) << left << "Reserved"
             << setw(12) << left << "Booked-By"
             << setw(13) << left << "Start-Date"
             << setw(12) << left << "End-Date" << endl;
        //遍历所有房间
        for (const auto& room : rooms) {
            //检查房间价格是否在指定区间内
            if (room.price >= minPrice && room.price <= maxPrice) {
                hasAvailableRooms = true;
                //输出符合条件的房间信息
                cout << setw(7) << left << room.roomNumber
                     << setw(9) << left << room.roomType
                     << "$" << setw(10) << left << room.price
                     << setw(11) << left << room.reservedCount
                     << setw(10) << left << room.bookedBy
                     << setw(12) << left << room.startDate
                     << setw(12) << left << room.endDate << endl;
            }
        }
        //如果没有找到符合条件的房间，给出提示
        if (!hasAvailableRooms) {
            cout << "No available rooms found within the specified price range." << endl;
        }
    }
}

//setting（更改数据信息板块）
//用户更改用户名
void Change_Username (string newUsername) {
    vector<User> users = Load_Users();
    //检查新用户名是否已存在
    for (const auto& user : users) {
        if (user.username == newUsername) {
            cout << "This username already exists. \nPlease choose another one." << endl;
            return;
        }
    }
    //找到当前用户并更新用户名
    for (auto& user : users) {
        if (user.username == Now_User) {
            user.username = newUsername;
            break;
        }
    }
    Save_Users(users);
    //更新房间数据中的用户名
    vector<Room> rooms = Load_Rooms();
    for (auto& room : rooms) {
        if (room.bookedBy == Now_User) {
            room.bookedBy = newUsername;
        }
        for (auto& later : room.later_bookedBy) {
            if (later.first == Now_User) {
                later.first = newUsername;
            }
        }
    }
    Save_Rooms(rooms);
    //更新请求数据中的用户名
    vector<Request> requests;
    ifstream requestFile(RequestData);
    string line;
    while (getline(requestFile, line)) {
        Request request;
        size_t pos = 0;
        string token;
        vector<string> tokens;
        while (!line.empty()) {
            pos = line.find('|');
            if (pos != string::npos) {
                token = line.substr(0, pos);
                tokens.push_back(token);
                line.erase(0, pos + 1);
            } else {
                token = line;
                tokens.push_back(token);
                line.erase();
            }
        }
        request.requestNumber = stoi(tokens[0]);
        request.username = tokens[1];
        request.Top_up = stod(tokens[2]);
        request.requestDate = tokens[3];
        if (request.username == Now_User) {
            request.username = newUsername;
        }
        requests.push_back(request);
    }
    requestFile.close();
    ofstream outRequestFile(RequestData);
    for (const auto& request : requests) {
        outRequestFile << request.requestNumber << "|" << request.username << "|" << request.Top_up << "|"
                       << request.requestDate << endl;
    }
    outRequestFile.close();
    cout << "Username changed successfully." << endl;
    cout << "Please ReLogin." << endl;
    IsUserLogin = false;
}

//更改密码
void Change_Password (string newPassword) {
    vector<User> users = Load_Users();
    //找到当前用户并更新密码
    for (auto& user : users) {
        if (user.username == Now_User) {
            user.password = newPassword;
            break;
        }
    }
    //保存更新后的用户数据
    Save_Users(users);
    cout << "Password changed successfully." << endl;
    cout << "Please ReLogin." << endl;
    IsUserLogin = false;
}

//注销账号（慎用，特别是管理员账号）
void Delete_Account () {
    vector<User> users = Load_Users();
    for (auto it = users.begin(); it != users.end(); ++it) {
        if (it->username == Now_User) {
            users.erase(it);
            break;
        }
    }
    Save_Users(users);
    //移除房间数据中该用户的预订信息
    vector<Room> rooms = Load_Rooms();
    for (auto& room : rooms) {
        auto it = room.later_bookedBy.begin();
        while (it != room.later_bookedBy.end()) {
            if (it->first == Now_User) {
                it = room.later_bookedBy.erase(it);
                room.reservedCount--;
            } else {
                ++it;
            }
        }
        if (room.bookedBy == Now_User) {
            room.bookedBy = "None";
            room.startDate = GetCurrentDate();
            room.endDate = GetCurrentDate();
            room.reservedCount--;
            if (!room.later_bookedBy.empty()) {
                room.bookedBy = room.later_bookedBy.front().first;
                room.startDate = room.later_bookedBy.front().second.first;
                room.endDate = room.later_bookedBy.front().second.second;
                room.later_bookedBy.erase(room.later_bookedBy.begin());
            }
        }
    }
    Save_Rooms(rooms);
    //移除请求数据中该用户的请求信息
    vector<Request> requests;
    ifstream requestFile(RequestData);
    string line;
    while (getline(requestFile, line)) {
        Request request;
        size_t pos = 0;
        string token;
        vector<string> tokens;
        while (!line.empty()) {
            pos = line.find('|');
            if (pos != string::npos) {
                token = line.substr(0, pos);
                tokens.push_back(token);
                line.erase(0, pos + 1);
            } else {
                token = line;
                tokens.push_back(token);
                line.erase();
            }
        }
        request.requestNumber = stoi(tokens[0]);
        request.username = tokens[1];
        request.Top_up = stod(tokens[2]);
        request.requestDate = tokens[3];
        if (request.username != Now_User) {
            requests.push_back(request);
        }
    }
    requestFile.close();
    ofstream outRequestFile(RequestData);
    for (const auto& request : requests) {
        outRequestFile << request.requestNumber << "|" << request.username << "|" << request.Top_up << "|"
                       << request.requestDate << endl;
    }
    outRequestFile.close();
    cout << "Account deleted successfully." << endl;
    IsUserLogin = false;
}

//管理员功能
//添加房间
void Add_Room () {
    vector<Room> rooms = Load_Rooms();
    View_Rooms(Load_Rooms());
    Room newRoom;
    string input;
    cout << "Enter room number: ";
    cin >> input;
    try {
        int NewroomNumber = stoi(input);
    }
    catch (invalid_argument &) {
        cout << "Invalid_argument." << endl;
        return;
    }
    catch (out_of_range &) {
        cout << "Out of range." << endl;
        return;
    }
    catch (...) {
        cout << "Something else error." << endl;
        return;
    }
    int NewroomNumber = stoi(input);
    newRoom.roomNumber = NewroomNumber;
    // 检查房间号是否已存在
    for (const auto& room : rooms) {
        if (room.roomNumber == newRoom.roomNumber) {
            cout << "Room number already exists. Please choose another one." << endl;
            return;
        }
    }
    cout << "Enter room type (Single, Double, Suite, etc): ";
    cin >> newRoom.roomType;
    cout << "Enter room price per day: ";
    cin >> input;
    try {
        double NewroomPrice = stod(input);
    }
    catch (invalid_argument &) {
        cout << "Invalid_argument." << endl;
        return;
    }
    catch (out_of_range &) {
        cout << "Out of range." << endl;
        return;
    }
    catch (...) {
        cout << "Something else error." << endl;
        return;
    }
    double NewroomPrice = stod(input);
    newRoom.price = NewroomPrice;
    newRoom.reservedCount = 0;
    newRoom.bookedBy = "None";
    newRoom.startDate = GetCurrentDate();
    newRoom.endDate = GetCurrentDate();
    rooms.push_back(newRoom);
    Save_Rooms(rooms);
    cout << "Room added successfully." << endl;
}

//删除房间
void Delete_Room() {
    vector<Room> rooms = Load_Rooms();
    if (rooms.empty()) {
        cout << "Room not found." << endl;
        return;
    }
    View_Rooms(Load_Rooms());
    string input;
    cout << "Enter the room number to delete: ";
    cin >> input;
    try {
        int roomNumber = stoi(input);
    }
    catch (invalid_argument &) {
        cout << "Invalid_argument." << endl;
        return;
    }
    catch (out_of_range &) {
        cout << "Out of range." << endl;
        return;
    }
    catch (...) {
        cout << "Something else error." << endl;
        return;
    }
    int roomNumber = stoi(input);
    string choose;
    cout << "Really (y/n)? : ";
    cin >> choose;
    if (choose != "Y" && choose != "y") return;
    for (auto it = rooms.begin(); it != rooms.end(); ++it) {
        if (it->roomNumber == roomNumber) {
            rooms.erase(it);
            Save_Rooms(rooms);
            cout << "Room deleted successfully." << endl;
            return;
        }
    }
    cout << "Room not found." << endl;
}

//修改房间信息
void Modify_Room () {
    vector<Room> rooms = Load_Rooms();
    if (rooms.empty()) {
        cout << "Room not found." << endl;
        return;
    }
    View_Rooms(Load_Rooms());
    string input;
    cout << "Enter the room number to modify: ";
    cin >> input;
    try {
        int roomNumber = stoi(input);
    }
    catch (invalid_argument &) {
        cout << "Invalid_argument." << endl;
        return;
    }
    catch (out_of_range &) {
        cout << "Out of range." << endl;
        return;
    }
    catch (...) {
        cout << "Something else error." << endl;
        return;
    }
    int roomNumber = stoi(input);
    for (auto& room : rooms) {
        if (room.roomNumber == roomNumber) {
            cout << "Enter new room type (Single, Double, Suite, etc): ";
            cin >> room.roomType;
            cout << "Enter new room price per day: ";
            cin >> input;
            try {
                double NewroomPrice = stod(input);
            }
            catch (invalid_argument &) {
                cout << "Invalid_argument." << endl;
                return;
            }
            catch (out_of_range &) {
                cout << "Out of range." << endl;
                return;
            }
            catch (...) {
                cout << "Something else error." << endl;
                return;
            }
            double NewroomPrice = stod(input);
            room.price = NewroomPrice;
            Save_Rooms(rooms);
            cout << "Room information modified successfully." << endl;
            return;
        }
    }
    cout << "Room not found." << endl;
}

//所有用户信息
void View_Users() {
    vector<User> users = Load_Users();
    if (users.empty()) {
        cout << "No users found." << endl;
        return;
    }
    cout << setw(11) << left << "Username"
         << setw(11) << left << "Password"
         << setw(16) << left << "Role"
         << setw(10) << left << "Balance" << endl;
    for (const auto& user : users) {
        cout << setw(11) << left << user.username
             << setw(11) << left << user.password
             << setw(16) << left << user.role
             << setw(10) << left << user.balance << endl;
    }
}

//添加用户
void Add_User () {
    View_Users();
    vector<User> users = Load_Users();
    string newUsername;
    cout << "Please enter the new user's username: ";
    cin >> newUsername;
    //检查用户名是否已存在
    for (const auto& user : users) {
        if (user.username == newUsername) {
            cout << "This username already exists. Please choose another one." << endl;
            return;
        }
    }
    string newPassword;
    cout << "Please enter the new user's password: ";
    cin >> newPassword;
    string newRole;
    cout << "Please enter the new user's role (Ordinary member, VIP member, Administrator): ";
    cin >> newRole;
    if (newRole != "Ordinary member" && newRole != "VIP member" && newRole != "Administrator") {
        cout << "Invalid role. Please enter a valid role (Ordinary member, VIP member, Administrator)." << endl;
        return;
    }
    string initialBalance_str;
    cout << "Please enter the new user's initial balance: $";
    cin >> initialBalance_str;
    try {
        double initialBalance = stoi(initialBalance_str);
    }
    catch (invalid_argument &) {
        cout << "Invalid_argument." << endl;
        return;
    }
    catch (out_of_range &) {
        cout << "Out of range." << endl;
        return;
    }
    catch (...) {
        cout << "Something else error." << endl;
        return;
    }
    double initialBalance = stoi(initialBalance_str);
    if (initialBalance < 0.0) {
        cout << "The initial balance cannot be negative." << endl;
        return;
    }
    User newUser;
    newUser.username = newUsername;
    newUser.password = newPassword;
    newUser.role = newRole;
    newUser.balance = initialBalance;
    users.push_back(newUser);
    Save_Users(users);
    cout << "New user " << newUsername << " has been added successfully." << endl;
}

//删除用户
void Delete_User () {
    View_Users();
    vector<User> users = Load_Users();
    string targetUsername;
    cout << "Please enter the username of the user you want to delete: ";
    cin >> targetUsername;
    string choose;
    cout << "Really (y/n)? : ";
    cin >> choose;
    if (choose != "Y" && choose != "y") return;
    //查找要删除的用户
    auto userIt = find_if(users.begin(), users.end(), [&targetUsername](const User& user) {
        return user.username == targetUsername;
    });
    //若未找到该用户，输出提示信息并返回
    if (userIt == users.end()) {
        cout << "User with username " << targetUsername << " not found." << endl;
        return;
    }
    //从用户列表中移除该用户
    users.erase(userIt);
    //将更新后的用户列表保存到文件
    Save_Users(users);
    //加载所有房间信息
    vector<Room> rooms = Load_Rooms();
    for (auto& room : rooms) {
        //移除该用户在房间后续预订列表中的信息
        auto laterIt = room.later_bookedBy.begin();
        while (laterIt != room.later_bookedBy.end()) {
            if (laterIt->first == targetUsername) {
                laterIt = room.later_bookedBy.erase(laterIt);
                room.reservedCount--;
            } else {
                ++laterIt;
            }
        }
        //若当前房间由该用户预订，将房间状态重置
        if (room.bookedBy == targetUsername) {
            room.bookedBy = "None";
            room.startDate = GetCurrentDate();
            room.endDate = GetCurrentDate();
            room.reservedCount--;
            //若有后续预订，将下一个预订提前
            if (!room.later_bookedBy.empty()) {
                room.bookedBy = room.later_bookedBy.front().first;
                room.startDate = room.later_bookedBy.front().second.first;
                room.endDate = room.later_bookedBy.front().second.second;
                room.later_bookedBy.erase(room.later_bookedBy.begin());
            }
        }
    }
    //将更新后的房间信息保存到文件
    Save_Rooms(rooms);
    //加载所有充值请求信息
    vector<Request> requests;
    ifstream requestFile(RequestData);
    string line;
    while (getline(requestFile, line)) {
        Request request;
        size_t pos = 0;
        string token;
        vector<string> tokens;
        while (!line.empty()) {
            pos = line.find('|');
            if (pos != string::npos) {
                token = line.substr(0, pos);
                tokens.push_back(token);
                line.erase(0, pos + 1);
            } else {
                token = line;
                tokens.push_back(token);
                line.erase();
            }
        }
        request.requestNumber = stoi(tokens[0]);
        request.username = tokens[1];
        request.Top_up = stod(tokens[2]);
        request.requestDate = tokens[3];
        requests.push_back(request);
    }
    requestFile.close();
    //移除与该用户相关的充值请求
    requests.erase(remove_if(requests.begin(), requests.end(), [&targetUsername](const Request& request) {
        return request.username == targetUsername;
    }), requests.end());
    //将更新后的充值请求信息保存到文件
    ofstream outRequestFile(RequestData);
    for (const auto& request : requests) {
        outRequestFile << request.requestNumber << "|" << request.username << "|" << request.Top_up << "|"
                       << request.requestDate << endl;
    }
    outRequestFile.close();
    cout << "User with username " << targetUsername << " has been deleted successfully." << endl;
}

//修改用户信息
void Modify_User () {
    View_Users();
    vector<User> users = Load_Users();
    string targetUsername;
    cout << "Please enter the username of the user you want to modify: ";
    cin >> targetUsername;
    //查找用户直接赋值it（呜呜呜呜早知道能这样就一直这样了）
    auto it = find_if(users.begin(), users.end(), [&targetUsername](const User& user) {
        return user.username == targetUsername;
    });
    if (it == users.end()) {
        cout << "User not found." << endl;
        return;
    }
    string choice;
    while (true) {
        cout << "\nWhat information do you want to modify?" << endl;
        cout << "1. Username" << endl;
        cout << "2. Password" << endl;
        cout << "3. Role" << endl;
        cout << "4. Balance" << endl;
        cout << "0. Exit modification" << endl;
        cout << "select >> ";
        cin >> choice;
        if (choice == "0") {
            break;
        } else if (choice == "1") {
            string newUsername;
            cout << "Please enter the new username: ";
            cin >> newUsername;
            //检查新用户名是否已存在
            for (const auto& user : users) {
                if (user.username == newUsername) {
                    cout << "This username already exists. Please choose another one." << endl;
                    continue;
                }
            }
            string oldUsername = it->username;
            it->username = newUsername;
            //更新房间数据中的用户名
            vector<Room> rooms = Load_Rooms();
            for (auto& room : rooms) {
                if (room.bookedBy == oldUsername) {
                    room.bookedBy = newUsername;
                }
                for (auto& later : room.later_bookedBy) {
                    if (later.first == oldUsername) {
                        later.first = newUsername;
                    }
                }
            }
            Save_Rooms(rooms);
            //更新请求数据中的用户名
            vector<Request> requests;
            ifstream requestFile(RequestData);
            string line;
            while (getline(requestFile, line)) {
                Request request;
                size_t pos = 0;
                string token;
                vector<string> tokens;
                while (!line.empty()) {
                    pos = line.find('|');
                    if (pos != string::npos) {
                        token = line.substr(0, pos);
                        tokens.push_back(token);
                        line.erase(0, pos + 1);
                    } else {
                        token = line;
                        tokens.push_back(token);
                        line.erase();
                    }
                }
                request.requestNumber = stoi(tokens[0]);
                request.username = tokens[1];
                request.Top_up = stod(tokens[2]);
                request.requestDate = tokens[3];
                if (request.username == oldUsername) {
                    request.username = newUsername;
                }
                requests.push_back(request);
            }
            requestFile.close();
            ofstream outRequestFile(RequestData);
            for (const auto& request : requests) {
                outRequestFile << request.requestNumber << "|" << request.username << "|" << request.Top_up << "|"
                               << request.requestDate << endl;
            }
            outRequestFile.close();
            cout << "Username modified successfully." << endl;
        } else if (choice == "2") {
            string newPassword;
            cout << "Please enter the new password: ";
            cin >> newPassword;
            it->password = newPassword;
            cout << "Password modified successfully." << endl;
        } else if (choice == "3") {
            string newRole;
            cout << "Please enter the new role (Ordinary member, VIP member, Administrator): ";
            cin >> newRole;
            if (newRole != "Ordinary member" && newRole != "VIP member" && newRole != "Administrator") {
                cout << "Invalid role. Please enter a valid role (Ordinary member, VIP member, Administrator)." << endl;
                continue;
            }
            it->role = newRole;
            cout << "Role modified successfully." << endl;
        } else if (choice == "4") {
            double newBalance;
            cout << "Please enter the new balance: $";
            cin >> newBalance;
            if (newBalance < 0) {
                cout << "Balance cannot be negative. Please enter a valid balance." << endl;
                continue;
            }
            it->balance = newBalance;
            cout << "Balance modified successfully." << endl;
        } else {
            cout << "Invalid choice. Please try again." << endl;
        }
    }
    //保存更新后的用户数据
    Save_Users(users);
}

//查看请求信息
void View_Requests () {
    vector<Request> requests;
    ifstream file(RequestData);
    string line;
    while (getline(file, line)) {
        Request request;
        size_t pos = 0;
        string token;
        vector<string> tokens;
        while (!line.empty()) {
            pos = line.find('|');
            if (pos != string::npos) {
                token = line.substr(0, pos);
                tokens.push_back(token);
                line.erase(0, pos + 1);
            } else {
                token = line;
                tokens.push_back(token);
                line.erase();
            }
        }
        request.requestNumber = stoi(tokens[0]);
        request.username = tokens[1];
        request.Top_up = stod(tokens[2]);
        request.requestDate = tokens[3];
        requests.push_back(request);
    }
    file.close();
    if (requests.empty()) {
        cout << "No requests found." << endl;
        return;
    }
    cout << setw(11) << left << "Number"
         << setw(13) << left << "Username"
            << setw(10) << left << "Amount"
            << setw(15) << left << "Request-Date" << endl;
    for (const auto& request : requests) {
        cout << setw(11) << left << request.requestNumber
             << setw(13) << left << request.username
             << "$" << setw(10) << left << request.Top_up
             << setw(15) << left << request.requestDate << endl;
    }
}

//处理订单
void Process_Request() {
    vector<Request> requests;
    ifstream file(RequestData);
    string line;
    while (getline(file, line)) {
        Request request;
        size_t pos = 0;
        string token;
        vector<string> tokens;
        while (!line.empty()) {
            pos = line.find('|');
            if (pos != string::npos) {
                token = line.substr(0, pos);
                tokens.push_back(token);
                line.erase(0, pos + 1);
            } else {
                token = line;
                tokens.push_back(token);
                line.erase();
            }
        }
        request.requestNumber = stoi(tokens[0]);
        request.username = tokens[1];
        request.Top_up = stod(tokens[2]);
        request.requestDate = tokens[3];
        requests.push_back(request);
    }
    file.close();
    if (requests.empty()) {
        return;
    }
    string input;
    cout << "Enter the request number to process: ";
    cin >> input;
    try {
        int requestNumber = stoi(input);
    }
    catch (invalid_argument &) {
        cout << "Invalid_argument." << endl;
        return;
    }
    catch (out_of_range &) {
        cout << "Out of range." << endl;
        return;
    }
    catch (...) {
        cout << "Something else error." << endl;
        return;
    }
    int requestNumber = stoi(input);
    auto it = find_if(requests.begin(), requests.end(), [requestNumber](const Request& req) {
        return req.requestNumber == requestNumber;
    });
    if (it == requests.end()) {
        cout << "Request not found." << endl;
        return;
    }
    char choice;
    cout << "Do you want to process this request (y/n)? ";
    cin >> choice;
    if (choice == 'y' || choice == 'Y') {
        //处理请求
        vector<User> users = Load_Users();
        for (auto& user : users) {
            if (user.username == it->username) {
                user.balance += it->Top_up;
                if (it->Top_up >= 1000 && user.role != "Administrator" && user.role != "VIP member") {
                    user.role = "VIP member";
                }
                break;
            }
        }
        Save_Users(users);
        cout << "Request processed successfully." << endl;
    } else {
        cout << "Request rejected." << endl;
    }
    //删除已处理或拒绝的请求
    requests.erase(it);
    ofstream outFile(RequestData);
    for (const auto& request : requests) {
        outFile << request.requestNumber << "|" << request.username << "|" << request.Top_up << "|"
                << request.requestDate << endl;
    }
    outFile.close();
}

//系统运行
//初始化数据文件（检测文件丢失或首次启动）
void System_Initialize () {
    //检测文件完整性
    ifstream user_data (UserData);
    ifstream room_data (RoomData);
    ifstream request_data (RequestData);
    if (!user_data) {
        //初始化用户数据文件
        ofstream user_data (UserData);
        if (user_data.is_open()) {
            user_data << "admin|admin|Administrator|999999999" << endl;
            user_data.close();
        }
        else IsDataRunning = false;
    } else {
        user_data.close();
    }
    if (!room_data) {
        //初始化房间数据文件
        ofstream room_data (RoomData);
        if (room_data.is_open()) {
            room_data << "101|Single|150|0|None|" << GetCurrentDate() << "|" << GetCurrentDate() << endl;
            room_data << "201|Double|250|0|None|" << GetCurrentDate() << "|" << GetCurrentDate() << endl;
            room_data << "301|Suite|1150|0|None|" << GetCurrentDate() << "|" << GetCurrentDate() << endl;
            room_data.close();
        }
        else IsDataRunning = false;
    } else {
        room_data.close();
    }
    if (!request_data) {
        //初始化请求数据文件
        ofstream request_data (RequestData);
        if (request_data.is_open()) {
            request_data.close();
        }
        else IsDataRunning = false;
    } else {
        request_data.close();
    }
}

//初始化数据（自动退房）
void System_Initialize_Room () {
    vector<Room> rooms = Load_Rooms();
    for (auto& room : rooms) {
        if (room.endDate < GetCurrentDate()) {
            if (!room.later_bookedBy.empty()) {
                //如果有后续预订，将后续预订提前
                room.bookedBy = room.later_bookedBy.front().first;
                room.startDate = room.later_bookedBy.front().second.first;
                room.endDate = room.later_bookedBy.front().second.second;
                room.later_bookedBy.erase(room.later_bookedBy.begin());
                room.reservedCount --;
            } else {
                //如果没有后续预订，将房间设置为未预订
                room.bookedBy = "None";
                room.startDate = GetCurrentDate();
                room.endDate = GetCurrentDate();
                room.reservedCount = 0;
            }
        }
    }
    Save_Rooms(rooms);
}

//登录
void Login() {
    while (true) {
        string username;
        string password;
        cout << "username >> ";
        cin >> username;
        cout << "password >> ";
        cin >> password;
        vector<User> users = Load_Users();
        for (const auto & user : users) {
            if (user.username == username && user.password == password) {
                Now_User = username;
                IsUserLogin = true;
                return;
            }
        }
        cout << "Invalid username or password." << endl;
        cout << "Try again or exit?" << endl;
        for (int j = 0; j < 42; j ++) {
            if (j == 8) cout << "1.Try again";
            else cout << " ";
        }
        cout << endl;
        for (int j = 0; j < 39; j ++) {
            if (j == 8) cout << "2.Exit";
            else cout << " ";
        }
        cout << endl;
        char n;
        cout << "select >> ";
        cin >> n;
        if (n == '1');
        else if (n == '2') return;
        else {
            cout << "Try again!" << endl;
        }
    }
}

//注册
void Register () {
    while (true) {
        string username;
        string password;
        cout << "username >> ";
        cin >> username;
        cout << "password >> ";
        cin >> password;
        vector<User> users = Load_Users();
        bool UserNameExist = false;
        for (const auto &user: users) {
            if (user.username == username) {
                cout << "Username already exist." << endl;
                cout << "Try again or exit?" << endl;
                UserNameExist = true;
                for (int j = 0; j < 42; j ++) {
                    if (j == 8) cout << "1.Try again";
                    else cout << " ";
                }
                cout << endl;
                for (int j = 0; j < 39; j ++) {
                    if (j == 8) cout << "2.Exit";
                    else cout << " ";
                }
                cout << endl;
                char n;
                cout << "select >> ";
                cin >> n;
                if (n == '1') break;
                else if (n == '2') return;
                else {
                    cout << "Try again!" << endl;
                    break;
                }
            }
        }
        if (!UserNameExist) {
            User NewUser;
            NewUser.username = username;
            NewUser.password = password;
            users.push_back(NewUser);
            Save_Users(users);
            cout << "Registration successful." << endl;
            cout << "Please Login." << endl;
            Login();
            return;
        }
    }
}

//副菜单功能
void Side_Menu (int order) {
    if (order == 1) {
        while (true) {
            system("cls");
            for (int i = 0; i < 48; i++) {
                if (i == 23) cout << "Hotel Management System";
                else cout << "_";
            }
            cout << endl;
            cout << "Welcome, " << Now_User << "! (" << View_Role() << ")" << endl;
            View_Balance();
            cout << "The discount you can enjoy is " << setprecision(0) << fixed
                 << (1.0 - View_Discount(View_Role())) * 100 << "%" << endl;
            cout << endl;
            for (int i = 0; i < 48; i++) {
                if (i == 28) cout << "1.Find-by-Date";
                else cout << " ";
            }
            cout << endl;
            for (int i = 0; i < 48; i++) {
                if (i == 28) cout << "2.Find-by-Type";
                else cout << " ";
            }
            cout << endl;
            for (int i = 0; i < 48; i++) {
                if (i == 28) cout << "3.Find-by-Price";
                else cout << " ";
            }
            cout << endl << endl;
            for (int j = 0; j < 65; j++) {
                if (j == 57) cout << GetCurrentDate();
                else cout << " ";
            }
            cout << endl;
            for (int i = 0; i < 55; i++) {
                if (i == 27) cout << "Welcome To Hotel";
                else cout << "_";
            }
            cout << endl;
            cout << "Do you want to find the available rooms (y/n)? : ";
            char choose;
            cin >> choose;
            if (choose == 'Y' || choose == 'y') {
                string n;
                cout << "select >> ";
                cin >> n;
                if (n == "1" || n == "2" || n == "3") {
                    Find_Rooms(stoi(n));
                } else {
                    cout << "Invalid_argument." << endl;
                }
                cout << "Do you want to exit (y/n)? : ";
                char choose;
                cin >> choose;
                if (choose == 'Y' || choose == 'y') {
                    break;
                }
            } else if (choose == 'N' || choose == 'n') {
                break;
            } else {
                cout << "Do you want to exit (y/n)? : ";
                char choose;
                cin >> choose;
                if (choose == 'Y' || choose == 'y') {
                    break;
                }
            }
        }
    } else if (order == 2) {
        while (true) {
            system("cls");
            for (int i = 0; i < 48; i++) {
                if (i == 23) cout << "Hotel Management System";
                else cout << "_";
            }
            cout << endl;
            cout << "Welcome, " << Now_User << "! (" << View_Role() << ")" << endl;
            View_Balance();
            cout << "The discount you can enjoy is " << setprecision(0) << fixed
                 << (1.0 - View_Discount(View_Role())) * 100 << "%" << endl;
            cout << endl;
            for (int j = 0; j < 39; j++) {
                if (j == 30) cout << "Room_List";
                else cout << " ";
            }
            cout << endl;
            View_Rooms(Load_Rooms());
            cout << endl;
            for (int j = 0; j < 65; j++) {
                if (j == 57) cout << GetCurrentDate();
                else cout << " ";
            }
            cout << endl;
            for (int i = 0; i < 55; i++) {
                if (i == 27) cout << "Welcome To Hotel";
                else cout << "_";
            }
            cout << endl;
            cout << "Do you want to book a room (y/n)? : ";
            char choose;
            cin >> choose;
            if (choose == 'Y' || choose == 'y') {
                string n;
                cout << "Please enter the room number : ";
                cin >> n;
                Book_Room(n);
                if (IsTradeCompleted) {
                    cout << "Booking successful!" << endl;
                    IsTradeCompleted = false;
                } else {
                    cout << "Booking cancelled." << endl;
                }
                cout << "Do you want to exit (y/n)? : ";
                char choose;
                cin >> choose;
                if (choose == 'Y' || choose == 'y') {
                    break;
                }
            } else if (choose == 'N' || choose == 'n') {
                break;
            } else {
                cout << "Do you want to exit (y/n)? : ";
                char choose;
                cin >> choose;
                if (choose == 'Y' || choose == 'y') {
                    break;
                }
            }
        }
        return;
    } else if (order == 3) {
        while (true) {
            system("cls");
            for (int i = 0; i < 48; i++) {
                if (i == 23) cout << "Hotel Management System";
                else cout << "_";
            }
            cout << endl;
            cout << "Welcome, " << Now_User << "! (" << View_Role() << ")" << endl;
            View_Balance();
            cout << "The discount you can enjoy is " << setprecision(0) << fixed
                 << (1.0 - View_Discount(View_Role())) * 100 << "%" << endl;
            cout << endl;
            for (int j = 0; j < 39; j++) {
                if (j == 29) cout << "Your_Rooms";
                else cout << " ";
            }
            cout << endl;
            View_Booked_Rooms(); //显示当前用户预订的房间
            cout << endl;
            for (int j = 0; j < 65; j++) {
                if (j == 57) cout << GetCurrentDate();
                else cout << " ";
            }
            cout << endl;
            for (int i = 0; i < 55; i++) {
                if (i == 27) cout << "Welcome To Hotel";
                else cout << "_";
            }
            cout << endl;
            cout << "Do you want to cancel a booking (y/n)? : ";
            char choose;
            cin >> choose;
            if (choose == 'Y' || choose == 'y') {
                string n;
                cout << "Please enter the room number : ";
                cin >> n;
                Cancel_Booking(n);
                if (IsTradeCompleted) {
                    IsTradeCompleted = false;
                } else {
                    cout << "Cancellation cancelled." << endl;
                }
                cout << "Do you want to exit (y/n)? : ";
                char choose;
                cin >> choose;
                if (choose == 'Y' || choose == 'y') {
                    break;
                }
            } else if (choose == 'N' || choose == 'n') {
                break;
            } else {
                cout << "Do you want to exit (y/n)? : ";
                char choose;
                cin >> choose;
                if (choose == 'Y' || choose == 'y') {
                    break;
                }
            }
        }
        return;
    } else if (order == 4) {
        while (true) {
            system("cls");
            for (int i = 0; i < 48; i++) {
                if (i == 23) cout << "Hotel Management System";
                else cout << "_";
            }
            cout << endl;
            cout << "Welcome, " << Now_User << "! (" << View_Role() << ")" << endl;
            View_Balance();
            cout << "The discount you can enjoy is " << setprecision(0) << fixed
                 << (1.0 - View_Discount(View_Role())) * 100 << "%" << endl;
            cout << endl;
            for (int j = 0; j < 39; j++) {
                if (j == 29) cout << "Your_Rooms";
                else cout << " ";
            }
            cout << endl;
            View_Booked_Rooms(); //显示当前用户预订的房间
            cout << endl;
            for (int j = 0; j < 65; j++) {
                if (j == 57) cout << GetCurrentDate();
                else cout << " ";
            }
            cout << endl;
            for (int i = 0; i < 55; i++) {
                if (i == 27) cout << "Welcome To Hotel";
                else cout << "_";
            }
            cout << endl;
            cout << "Do you want to exit (y/n)? : ";
            char choose;
            cin >> choose;
            if (choose == 'Y' || choose == 'y') {
                break;
            }
        }
    } else if (order == 5) {
        while (true) {
            system("cls");
            for (int i = 0; i < 48; i++) {
                if (i == 23) cout << "Hotel Management System";
                else cout << "_";
            }
            cout << endl;
            cout << "Welcome, " << Now_User << "! (" << View_Role() << ")" << endl;
            View_Balance();
            cout << "The discount you can enjoy is " << setprecision(0) << fixed
                 << (1.0 - View_Discount(View_Role())) * 100 << "%" << endl;
            cout << endl;
            for (int j = 0; j < 39; j++) {
                if (j == 28) cout << "Recharge_Rules";
                else cout << " ";
            }
            cout << endl;
            for (int j = 0; j < 70; j++) {
                if (j > 7 && j < 64) cout << "~";
                else cout << " ";
            }
            cout << endl;
            for (int j = 0; j < 70; j++) {
                if (j == 8) cout << "Welcome!";
                else cout << " ";
            }
            cout << endl;
            for (int j = 0; j < 70; j++) {
                if (j == 8) cout << "When you submit the top-up amount,";
                else cout << " ";
            }
            cout << endl;
            for (int j = 0; j < 18; j++) {
                if (j == 8) cout << "we will hand over the order to the administrator system.";
                else cout << " ";
            }
            cout << endl;
            for (int j = 0; j < 70; j++) {
                if (j == 8) cout << "So contact your administrator after topping up.";
                else cout << " ";
            }
            cout << endl;
            for (int j = 0; j < 18; j++) {
                if (j == 8) cout << "If the single deposit amount is greater than 1000,";
                else cout << " ";
            }
            cout << endl;
            for (int j = 0; j < 18; j++) {
                if (j == 8) cout << "You can choose to become a VIP, This doesn't cost extra!";
                else cout << " ";
            }
            cout << endl;
            for (int j = 0; j < 70; j++) {
                if (j > 7 && j < 64) cout << "~";
                else cout << " ";
            }
            cout << endl << endl;
            for (int j = 0; j < 65; j++) {
                if (j == 57) cout << GetCurrentDate();
                else cout << " ";
            }
            cout << endl;
            for (int i = 0; i < 55; i++) {
                if (i == 27) cout << "Welcome To Hotel";
                else cout << "_";
            }
            cout << endl;
            Recharge_Balance();
            if (IsTradeCompleted) {
                cout << "The recharge application was sent successfully!" << endl;
                IsTradeCompleted = false;
            } else {
                cout << "Application was canceled." << endl;
            }
            cout << "Do you want to exit (y/n)? : ";
            char choose;
            cin >> choose;
            if (choose == 'Y' || choose == 'y') {
                break;
            }
        }
    } else if (order == 6) {
        while (true) {
            system("cls");
            for (int i = 0; i < 26; i++) {
                if (i == 12) cout << "Hotel Management System";
                else cout << "_";
            }
            cout << endl;
            cout << "Welcome, " << Now_User << "! (" << View_Role() << ")" << endl;
            View_Balance();
            cout << "The discount you can enjoy is " << setprecision(0) << fixed
                 << (1.0 - View_Discount(View_Role())) * 100 << "%" << endl;
            cout << endl;
            for (int j = 0; j < 39; j++) {
                if (j == 15) cout << "1.Change-username";
                else cout << " ";
            }
            cout << endl;
            for (int j = 0; j < 40; j ++) {
                if (j == 15) cout << "2.Change-password";
                else cout << " ";
            }
            cout << endl;
            for (int j = 0; j < 40; j ++) {
                if (j == 15) cout << "3.Delete-account";
                else cout << " ";
            }
            cout << endl;
            for (int j = 0; j < 40; j ++) {
                if (j == 15) cout << "0.Exit";
                else cout << " ";
            }
            cout << endl;
            for (int j = 0; j < 40; j ++) {
                if (j == 36) cout << GetCurrentDate();
                else cout << " ";
            }
            cout << endl;
            for (int i = 0; i < 33; i ++) {
                if (i == 16) cout << "Welcome To Hotel";
                else cout << "_";
            }
            cout << endl;
            char n;
            cout << "select >> ";
            cin >> n;
            string rename;
            string repassword;
            string accepted;
            switch (n) {
                case '0':
                    return;
                case '1':
                    cout << "Please enter 'yes' to continue : ";
                    cin >> accepted;
                    if (accepted != "yes") break;
                    cout << "Please enter the username to be changed : ";
                    cin >> rename;
                    Change_Username(rename);
                    break;
                case '2':
                    cout << "Please enter 'yes' to continue : ";
                    cin >> accepted;
                    if (accepted != "yes") break;
                    cout << "Please enter the password to be changed : ";
                    cin >> repassword;
                    Change_Password(repassword);
                    break;
                case '3':
                    system("color 4");
                    cout << "This will delete all of your personal data!" << endl;
                    cout << "Please enter 'yes' to continue : ";
                    cin >> accepted;
                    if (accepted != "yes") break;
                    cout << "Really?" << endl;
                    cout << "Please enter 'yes' to continue : ";
                    cin >> accepted;
                    if (accepted != "yes") break;
                    Delete_Account();
                    system("color 6");
                    cout << "Thank you for your company!" << endl;
                    cout << "May one day you be able to reunite with your significant other!" << endl;
                    system("pause");
                    break;
                default:
                    break;
            }
            char choose;
            cout << "Do you want to exit (y/n)? : ";
            cin >> choose;
            if (IsUserLogin && (choose == 'Y' || choose == 'y')) {
                system("color 07");
                break;
            } else if (!IsUserLogin && (choose == 'Y' || choose == 'y')) {
                system("color 07");
                break;
            } else if (!IsUserLogin && (choose == 'N' || choose == 'n')) {
                cout << "You have changed something, and you should quit." << endl;
                system("pause");
                system("color 07");
                break;
            } else {
                system("color 07");
            }
        }
    } else if (order == 7) {
        while (true) {
            system("cls");
            for (int i = 0; i < 26; i++) {
                if (i == 12) cout << "Hotel Management System";
                else cout << "_";
            }
            cout << endl;
            cout << "Welcome, " << Now_User << "! (" << View_Role() << ")" << endl;
            cout << endl;
            for (int j = 0; j < 39; j++) {
                if (j == 17) cout << "1.Add-Room";
                else cout << " ";
            }
            cout << endl;
            for (int j = 0; j < 39; j++) {
                if (j == 17) cout << "2.Delete-Room";
                else cout << " ";
            }
            cout << endl;
            for (int j = 0; j < 39; j++) {
                if (j == 17) cout << "3.Modify-Room";
                else cout << " ";
            }
            cout << endl;
            for (int j = 0; j < 39; j++) {
                if (j == 17) cout << "4.Add-User";
                else cout << " ";
            }
            cout << endl;
            for (int j = 0; j < 39; j++) {
                if (j == 17) cout << "5.Delete-User";
                else cout << " ";
            }
            cout << endl;
            for (int j = 0; j < 39; j++) {
                if (j == 17) cout << "6.Modify-User";
                else cout << " ";
            }
            cout << endl;
            for (int j = 0; j < 39; j++) {
                if (j == 17) cout << "7.Process-Request";
                else cout << " ";
            }
            cout << endl;
            for (int j = 0; j < 40; j++) {
                if (j == 17) cout << "0.Exit";
                else cout << " ";
            }
            cout << endl;
            for (int j = 0; j < 40; j++) {
                if (j == 36) cout << GetCurrentDate();
                else cout << " ";
            }
            cout << endl;
            for (int i = 0; i < 33; i++) {
                if (i == 16) cout << "Welcome To Hotel";
                else cout << "_";
            }
            cout << endl;
            char n;
            cout << "select >> ";
            cin >> n;
            switch (n) {
                case '0':
                    return;
                case '1':
                    Add_Room();
                    break;
                case '2':
                    Delete_Room();
                    break;
                case '3':
                    Modify_Room();
                    break;
                case '4':
                    Add_User();
                    break;
                case '5':
                    Delete_User();
                    break;
                case '6':
                    Modify_User();
                    break;
                case '7':
                    View_Requests();
                    Process_Request();
                    break;
                default:
                    break;
            }
            cout << "Do you want to exit (y/n)? : ";
            char choose;
            cin >> choose;
            if (choose == 'Y' || choose == 'y') {
                break;
            }
        }
    }
}

//主菜单功能
void Main_Menu () {
    for (int i = 0; i < 26; i ++) {
        if (i == 12) cout << "Hotel Management System";
        else cout << "_";
    }
    cout << endl;
    cout << "Welcome, " << Now_User << "! (" << View_Role() << ")" << endl;
    View_Balance();
    cout << "The discount you can enjoy is " << setprecision(0) << fixed << (1.0 - View_Discount(View_Role())) * 100 << "%" << endl;
    cout << endl;
    for (int j = 0; j < 39; j ++) {
        if (j == 18) cout << "1.Available";
        else cout << " ";
    }
    cout << endl;
    for (int j = 0; j < 39; j ++) {
        if (j == 18) cout << "2.Check-In";
        else cout << " ";
    }
    cout << endl;
    for (int j = 0; j < 38; j ++) {
        if (j == 18) cout << "3.Check-Out";
        else cout << " ";
    }
    cout << endl;
    for (int j = 0; j < 40; j ++) {
        if (j == 18) cout << "4.My-room";
        else cout << " ";
    }
    cout << endl;
    for (int j = 0; j < 39; j ++) {
        if (j == 18) cout << "5.Recharge";
        else cout << " ";
    }
    cout << endl;
    for (int j = 0; j < 40; j ++) {
        if (j == 18) cout << "6.Setting";
        else cout << " ";
    }
    cout << endl;
    if (View_Role() == "Administrator") {
        for (int j = 0; j < 33; j ++) {
            if (j == 18) cout << "7.Administration";
            else cout << " ";
        }
        cout << endl;
    }
    for (int j = 0; j < 40; j ++) {
        if (j == 18) cout << "0.Log-Out";
        else cout << " ";
    }
    cout << endl << endl;
    for (int j = 0; j < 40; j ++) {
        if (j == 36) cout << GetCurrentDate();
        else cout << " ";
    }
    cout << endl;
    for (int i = 0; i < 33; i ++) {
        if (i == 16) cout << "Welcome To Hotel";
        else cout << "_";
    }
    cout << endl;
    char n;
    cout << "select >> ";
    cin >> n;
    switch (n) {
        case '0':
            IsUserLogin = false;
            break;
        case '1':
            Side_Menu(1);
            system("cls");
            Main_Menu();
            if (!IsUserLogin) break;
        case '2':
            Side_Menu(2);
            system("cls");
            Main_Menu();
            if (!IsUserLogin) break;
        case '3':
            Side_Menu(3);
            system("cls");
            Main_Menu();
            if (!IsUserLogin) break;
        case '4':
            Side_Menu(4);
            system("cls");
            Main_Menu();
            if (!IsUserLogin) break;
        case '5':
            Side_Menu(5);
            system("cls");
            Main_Menu();
            if (!IsUserLogin) break;
        case '6':
            Side_Menu(6);
            system("cls");
            if (!IsUserLogin) break;
            Main_Menu();
            if (!IsUserLogin) break;
        case '7':
            if (View_Role() == "Administrator") {
                Side_Menu(7);
                system("cls");
                Main_Menu();
                if (!IsUserLogin) break;
            } else {
                system("cls");
                Main_Menu();
                if (!IsUserLogin) break;
            }
        default:
            system("cls");
            Main_Menu();
            if (!IsUserLogin) break;
    }
}

//登录菜单功能
void Login_Menu () {
    for (int i = 0; i < 26; i ++) {
        if (i == 12) cout << "Hotel Management System";
        else cout << "_";
    }
    cout << endl << endl;
    for (int j = 0; j < 42; j ++) {
        if (j == 19) cout << "1.Login";
        else cout << " ";
    }
    cout << endl;
    for (int j = 0; j < 39; j ++) {
        if (j == 19) cout << "2.register";
        else cout << " ";
    }
    cout << endl;
    for (int j = 0; j < 43; j ++) {
        if (j == 19) cout << "0.exit";
        else cout << " ";
    }
    cout << endl;
    for (int j = 0; j < 40; j ++) {
        if (j == 36) cout << GetCurrentDate();
        else cout << " ";
    }
    cout << endl;
    for (int i = 0; i < 33; i ++) {
        if (i == 16) cout << "Welcome To Hotel";
        else cout << "_";
    }
    cout << endl;
    char n;
    cout << "select >> ";
    cin >> n;
    switch (n) {
        case '1':
            Login();
            if (!IsUserLogin) {
                system("cls");
                Login_Menu();
            }
            if (!IsUserLogin) break;
            system("cls");
            Main_Menu();
            system("cls");
            Login_Menu();
            break;
        case '2':
            Register();
            if (!IsUserLogin) {
                system("cls");
                Login_Menu();
            }
            if (!IsUserLogin) break;
            system("cls");
            Main_Menu();
            system("cls");
            Login_Menu();
            break;
        case '0':
            break;
        default:
            system("cls");
            Login_Menu();
    }
}

//主函数
int main() {
    system("color 07");
    System_Initialize();
    if (!IsDataRunning) System_Initialize();
    System_Initialize_Room();
    Login_Menu();
    return 0;
}
