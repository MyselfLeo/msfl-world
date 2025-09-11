//
// Created by leo on 8/18/25.
//

#ifndef LOGS_HPP
#define LOGS_HPP

#define wrldInfo(txt) (std::cout << "[wrld:info] " << txt << std::endl)
#define wrldError(txt) (std::cerr << "[wrld:error] " << txt << std::endl)
#define wrldVar(variable) (std::cout << "[wrld:var] " << #variable << " = " << variable << std::endl)
#endif // LOGS_HPP
