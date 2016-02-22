GLIBC_2.2.5 {
    global:
        pthread_create;
        pthread_cond_wait;
        pthread_cond_broadcast;
        pthread_cond_signal;
	pthread_mutex_lock;
    local:
        *;
};

GLIBC_2.3.2 {
    global:
        pthread_cond_wait;
        pthread_cond_broadcast;
        pthread_cond_signal;
} GLIBC_2.2.5;

GOMP_1.0 {
    global:
        GOMP_barrier;
};
