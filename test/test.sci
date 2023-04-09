// This program loads and plots the output files from the systemTest program.

fd1 = mopen("original.dat");
fd2 = mopen("noise.dat");
fd3 = mopen("tainted.dat");
fd4 = mopen("processed.dat");

x = mget(20000,"f",fd1);
y = mget(20000,"f",fd2);
z = mget(20000,"f",fd3);
k = mget(20000,"f",fd4);

mclose(fd1);
mclose(fd2);
mclose(fd3);
mclose(fd4);

subplot(411);
plot(x(1000:2000));

subplot(412);
plot(y(1000:2000));

subplot(413);
plot(z(1000:2000));

subplot(414);
plot(k(1000:2000));

