#include <cstdint>


enum class Side {buy, sell};
enum class OrderType {limit, market};


struct Order {
    uint64_t id;
    Side side;
    OrderType type;
    double price; // not used for market orders
    uint32_t quantity;
};
