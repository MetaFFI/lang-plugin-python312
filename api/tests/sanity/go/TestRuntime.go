package TestRuntime

import (
	"strings"
	"time"
	"fmt"
)

func HelloWorld() {
	println("Hello World, From Go!")
}

func ReturnsAnError(){
	panic("An error from ReturnsAnError")
}

func DivIntegers(x int, y int) float32{

	if y == 0{
		panic("Divisor is 0")
	}

	return float32(x) / float32(y)
}

func JoinStrings(arrs []string) string{
	return strings.Join(arrs, ",")
}

const FiveSeconds = time.Second*5
func WaitABit(d time.Duration) error{
	fmt.Printf("Waiting for %v\n", d)
	return nil
}

type TestMap struct{
	m map[string]interface{}
	Name string
}

func NewTestMap() *TestMap{
	return &TestMap{ 
		m: make(map[string]interface{}),
		Name: "TestMap Name",
	}
}

func (this *TestMap) Set(k string, v interface{}){
	fmt.Printf("Setting: %T\n", v)
	this.m[k] = v
}

func (this *TestMap) Get(k string) interface{}{
	v := this.m[k]
	fmt.Printf("Getting: %T\n", v)
	return v
}

func (this *TestMap) Contains(k string) bool{
	_, found := this.m[k]
	return found
}
