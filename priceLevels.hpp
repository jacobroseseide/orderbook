// Add an order to specific level
// Remove an order by ID
// Get total quantity (sum of all order quantities)
// Check if empty

// in each price level object, there is the specific price, and a vector of order objects
#pragma once
#include "order.hpp"
#include <vector>


class PriceLevel {
private:
    double price;
    std::vector<Order> orders;

public:

    PriceLevel(double p) { // pricelevel constructor
        price = p;
    }

    double getPrice() { // price getter
        return price;
    }
    bool isEmpty() { // returns true if empty
        return orders.empty(); 
    }

    // get total number of shares at price level
    uint32_t getTotalQuantity() {
        // loop through orders vector
        // create int variable to track total
        // each iteration add orders[i].quantity to total
        uint32_t total = 0;

        for (const Order& o : orders) {
            total += o.quantity;
        }

        return total;
    }

    void addOrder(Order o) {
        orders.push_back(o); // add specific order to specific price level
    }

    bool deleteOrder(uint64_t id) {

        for (size_t i=0; i<orders.size(); i++) { // find specific id of order to delete
            if (orders[i].id == id) {
                orders.erase(orders.begin() + i);
                return true;
            }
        }
        return false;
    }


};
