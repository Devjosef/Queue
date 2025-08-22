#include "optimized_queue.hpp"
#include <iostream>
#include <cassert>
#include <vector>

void test_empty_queue_operations() {
    std::cout << "Testing empty queue operations...\n";
    
    OptimizedQueue<int> queue(10);
    int value;
    
    assert(queue.empty());
    assert(queue.size() == 0);
    assert(!queue.try_pop(value));
    
    std::cout << "✓ Empty queue operations passed\n";
}

void test_single_element_queue() {
    std::cout << "Testing single element queue...\n";
    
    OptimizedQueue<int> queue(1);
    
    assert(queue.push(42));
    assert(queue.size() == 1);
    assert(!queue.empty());
    
    int value;
    assert(queue.try_pop(value));
    assert(value == 42);
    assert(queue.empty());
    
    std::cout << "✓ Single element queue passed\n";
}

void test_queue_capacity_limits() {
    std::cout << "Testing queue capacity limits...\n";
    
    OptimizedQueue<int> queue(3);
    
    assert(queue.push(1));
    assert(queue.push(2));
    assert(queue.push(3));
    assert(!queue.push(4)); // Should fail
    
    int value;
    assert(queue.try_pop(value) && value == 1);
    assert(queue.push(4)); // Should succeed now
    
    std::cout << "✓ Queue capacity limits passed\n";
}

void test_zero_capacity_queue() {
    std::cout << "Testing zero capacity queue...\n";
    
    OptimizedQueue<int> queue(0);
    assert(!queue.push(1));
    assert(queue.empty());
    
    std::cout << "✓ Zero capacity queue passed\n";
}

void test_batch_operations_edge_cases() {
    std::cout << "Testing batch operations edge cases...\n";
    
    OptimizedQueue<int> queue(5, 3);
    std::vector<int> batch;
    
    // Empty batch pop
    assert(!queue.try_pop_batch(batch));
    assert(batch.empty());
    
    // Partial batch
    queue.push(1);
    queue.push(2);
    assert(queue.try_pop_batch(batch));
    assert(batch.size() == 2);
    
    std::cout << "✓ Batch operations edge cases passed\n";
}

int main() {
    std::cout << "Queue Edge Case Tests\n";
    std::cout << "====================\n\n";
    
    test_empty_queue_operations();
    test_single_element_queue();
    test_queue_capacity_limits();
    test_zero_capacity_queue();
    test_batch_operations_edge_cases();
    
    std::cout << "\nAll queue edge case tests passed!\n";
    return 0;
}
