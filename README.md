# threadpool
thread pool
待完善，经测试不可重入任务由自己维护锁，要比池本身提供锁的性能要高。
所以目前先使用自己维护锁的策略，池只保证生产者拿task生产时的安全性
能保证任务的原子性，但不保证不会被中途打断（向终端输出字符时可能会出现乱序输出，这种任务可以自己维护锁，粒度会比较小，性能也不会太差）