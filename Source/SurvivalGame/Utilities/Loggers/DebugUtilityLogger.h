#pragma once

#include "Engine/GameEngine.h"

#define printText(Text) if(GEngine){GEngine->AddOnScreenDebugMessage(-1, 20.0f , FColor::Yellow, TEXT(Text),true);}

//printText("Some message");


#define print(Format, ...) if(GEngine){GEngine->AddOnScreenDebugMessage(1, 2.0f, FColor::Yellow, FString::Printf(TEXT(Format), ##__VA_ARGS__), false);}

//float Yaw;
//print("var value is: %f, Yaw);

//int num;
//print("int value is: %d, num);

//string TestString(TEXT("TestString"));
//print("string value is: %s, *TestString);


#define SuperLog(Format, Duration, Color, ...) if(GEngine){GEngine->AddOnScreenDebugMessage(-1, Duration, Color , FString::Printf(TEXT(Format), ##__VA_ARGS__), false);}
/**
 * 
	float TestValue = 100.0f;
	SuperLog("This is a test: %f", 30.0f, FColor::Green, TestValue);
	
	or

	float TestFloat = 100.0f;
	int TestInt = 50;
	string TestString(TEXT("TestString"));
	SuperLog("This is a test: %f, %d, %s", 30.0f, FColor::Green, TestFloat, TestInt, *TestString);
	SuperLog("Location: %s", 30.0f, FColor::Green, *MyCharacterRef->GetActorLocation().ToString());
	SuperLog("Name: %s", 60.0f, FColor::Green, *GetNameSafe(GetPawn()));
	SuperLog("Location: %s", 60.0f, FColor::Green, *GetPawn()->GetActorLocation().ToString());
 */