#include <vector>
#include <iostream>
#include "order.hpp"


// Private vector for buy orders, vector for sell orders

class OrderBook {
private: 
    std::vector<Order> buy_orders;
    std::vector<Order> sell_orders;

public:
    void addOrder(Order o) {
        if (o.side == Side::buy) {
            buy_orders.push_back(o);
        } else {
            sell_orders.push_back(o);
        }
    }

    void printBook() {
        for (const Order& o: buy_orders) {
            std::cout << "BUY " << o.quantity << " at " << o.price << std::endl;
        }
        for (const Order& o : sell_orders) {
            std::cout << "SELL " << o.quantity << " at " << o.price << std::endl;
        }

    }

};


// BUY: 100 shares @ 50.25
// BUY: 50 shares @ 50.00
// SELL: 75 shares @ 51.00
