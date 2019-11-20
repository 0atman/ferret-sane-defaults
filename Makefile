build:
	java -jar `which ferret` --release -i f.clj
	g++ -static -std=c++11 -pthread f.cpp -o app
	./app

clean:
	rm *.cpp

watch:
	java -jar ~/.scripts/bin/ferret -wi f.clj

ast:
	java -jar ~/.scripts/bin/ferret --ast -i f.clj
