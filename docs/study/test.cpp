// #include <stdio.h>

// int main(void){
//     printf("Hello World\n");
//     return 0;
// }

// #include <iostream>

// int main(void){
//     std::cout << "Hello 2 World\n";
//     return 0;
// }

// #include <iostream>

// int main(void){
//     const char* s = "Hello 3 World";
//     std::cout << s << "\n";
//     return 0;
// }

// #include <iostream>
// #include <string>

// int main(void){
//     std::string s = "Hello 4 World";
//     std::cout << s << "\n";
//     return 0;
// }


// ---------------------------------------------

// #include <iostream>
// #include <string>

// int main(void){
//     std::string s = "Hello World";
//     std::cout << s << "\n";
//     std::cout << s.size() << "\n";
//     return 0;
// }
// C++では「型」＝「データ＋使い方（関数）」がセット
// std::stringは型でもあり、クラスでもある。

// #include <iostream>
// #include <string>

// int main(void){
//     std::string s = "Hello World";
//     std::string sub = s.substr(0, 5);
//     std::cout << sub << "\n";
//     return 0;
// }


// #include <iostream>
// #include <string>

// int main(void){
//     std::string s = "Hello World";
//     size_t pos = s.find("World");

//     std::cout << pos << "\n";
//     return 0;
// }


// #include <iostream>
// #include <string>

// int main(void){
//     std::string line = "rule_distance: 50.0";

//     size_t pos = line.find(':');

//     std::string key = line.substr(0, pos);
//     std::string value = line.substr(pos + 1);

//     std::cout << key << "\n";
//     std::cout << value << "\n";

//     return 0;
// }



// =====================================================
// =                    LEVEL 3                        =
// =====================================================
// #include <iostream>
// #include <string>

// void print(std::string s){
//     std::cout << s << "\n";
// }
// void print(std::string& s){
//     std::cout << s << "\n";
// }


// int main(){
//     std::string a = "Helloooo";
//     print(a);
// }

// ---

// void print(std::string& s){
//     s = "Changed!";
// }
// void print(const std::string& s){
//     std::cout << s << "\n";
// }


// int main(){
//     std::string a = "Hello";
//     print(a);
//     std::cout << a << "\n";
// }


// ---



// =====================================================
// =                    LEVEL 4                        =
// =====================================================

// #include <iostream>

// int main(){
//     try {
//         std::cout << "Start\n";
//         throw 1;
//         std::cout << "End\n";
//     } catch(int e) {
//         std::cout << "Caught error\n";
//     }
// }



// =====================================================
// =                    LEVEL 6                        =
// =====================================================

#include <vector>
#include <iostream>

int main(){
    std::vector<int> v;

    v.push_back(10);
    v.push_back(20);
    v.push_back(30);

    std::cout << v[0] << "\n";
}