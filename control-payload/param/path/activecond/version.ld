GLIBC_2.2.5 {
    global:
        pthread_cond_wait;
        pthread_cond_timedwait;
	pthread_cond_signal;
	pthread_cond_broadcast;
	pthread_cond_destroy;
	pthread_mutex_lock;
	pthread_mutex_trylock;
	pthread_mutex_unlock;
    local:
        *;
};

GLIBC_2.3.2 {
    global:
        pthread_cond_wait;
        pthread_cond_timedwait;
	pthread_cond_signal;
	pthread_cond_broadcast;
	pthread_cond_destroy;
} GLIBC_2.2.5;

GOMP_1.0 {
    global:
        GOMP_barrier;
};
