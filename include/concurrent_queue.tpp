template <typename T>
void ConcurrentQueue<T>::push(const T& value) {
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push(value);
    }
    m_cond_var.notify_one();
}

template <typename T>
bool ConcurrentQueue<T>::try_pop(T& result) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_queue.empty()) {
        return false;
    }
    result = m_queue.front();
    m_queue.pop();
    return true;
}

template <typename T>
void ConcurrentQueue<T>::wait_and_pop(T& result) {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_cond_var.wait(lock, [this] { return !m_queue.empty(); });
    result = m_queue.front();
    m_queue.pop();
}

template <typename T>
bool ConcurrentQueue<T>::empty() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_queue.empty();
}
