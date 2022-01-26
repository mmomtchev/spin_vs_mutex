LDFLAGS=-lpthread

all: mutex tas_lock tas_lock_relaxed ttas_lock amd_lock

clean:
	rm -f mutex tas_lock tas_lock_relaxed ttas_lock amd_lock

mutex: test.cc spinlock.h
	g++ -O3 -o mutex -DMUTEX test.cc $(LDFLAGS)

tas_lock: test.cc spinlock.h
	g++ -O3 -o tas_lock -DTASLOCK test.cc $(LDFLAGS)

tas_lock_relaxed: test.cc spinlock.h
	g++ -O3 -o tas_lock_relaxed -DTASLOCK_RELAXED test.cc $(LDFLAGS)

ttas_lock: test.cc spinlock.h
	g++ -O3 -o ttas_lock -DTTASLOCK test.cc $(LDFLAGS)

amd_lock: test.cc spinlock.h
	g++ -O3 -o amd_lock -DAMDLOCK test.cc $(LDFLAGS)
