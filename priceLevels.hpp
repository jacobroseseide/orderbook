// Add an order to specific level
// Remove an order by ID
// Get total quantity (sum of all order quantities)
// Check if empty

// in each price level object, there is the specific price, and a vector of order objects
#pragma once
#include "order.hpp"
#include <vector>

class PriceLevel
{
private:
    double price;
    std::vector<Order> orders;

public:
    PriceLevel(double p)
    { // pricelevel constructor
        price = p;
    }

    double getPrice() const
    { // price getter
        return price;
    }
    bool isEmpty() const
    { // returns true if empty
        return orders.empty();
    }

    // get total number of shares at price level
    uint32_t getTotalQuantity() const
    {
        // loop through orders vector
        // create int variable to track total
        // each iteration add orders[i].quantity to total
        uint32_t total = 0;

        for (const Order &o : orders)
        {
            total += o.quantity;
        }

        return total;
    }

    void addOrder(Order o)
    {
        orders.push_back(o); // add specific order to specific price level
    }

    bool deleteOrder(uint64_t id)
    {

        for (size_t i = 0; i < orders.size(); i++)
        { // find specific id of order to delete
            if (orders[i].id == id)
            {
                orders.erase(orders.begin() + i);
                return true;
            }
        }
        return false;
    }

    // find an order by id and set its quantity; returns false if not found
    bool modifyOrder(uint64_t id, uint32_t new_quantity)
    {
        for (Order &o : orders)
        {
            if (o.id == id)
            {
                o.quantity = new_quantity;
                return true;
            }
        }
        return false;
    }

    // consume up to `qty` shares from the front of this level (FIFO); returns shares actually consumed
    uint32_t consume(uint32_t qty)
    {
        uint32_t consumed = 0;
        while (!orders.empty() && consumed < qty)
        {
            uint32_t available = orders.front().quantity;
            uint32_t take = std::min(available, qty - consumed);
            consumed += take;
            orders.front().quantity -= take;
            if (orders.front().quantity == 0)
                orders.erase(orders.begin());
        }
        return consumed;
    }
};
