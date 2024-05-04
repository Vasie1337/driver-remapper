#pragma once
#include <iostream>

struct FVector
{
public:
	using UnderlayingType = float;                                                                   // 0x0000(0x0008)(NOT AUTO-GENERATED PROPERTY)

	float                                         X;                                                 // 0x0000(0x0004)(Edit, BlueprintVisible, ZeroConstructor, SaveGame, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	float                                         Y;                                                 // 0x0004(0x0004)(Edit, BlueprintVisible, ZeroConstructor, SaveGame, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	float                                         Z;                                                 // 0x0008(0x0004)(Edit, BlueprintVisible, ZeroConstructor, SaveGame, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)

public:
	FVector& Normalize()
	{
		*this /= Magnitude();
		return *this;
	}
	FVector& operator*=(const FVector& Other)
	{
		*this = *this * Other;
		return *this;
	}
	FVector& operator*=(float Scalar)
	{
		*this = *this * Scalar;
		return *this;
	}
	FVector& operator+=(const FVector& Other)
	{
		*this = *this + Other;
		return *this;
	}
	FVector& operator-=(const FVector& Other)
	{
		*this = *this - Other;
		return *this;
	}
	FVector& operator/=(const FVector& Other)
	{
		*this = *this / Other;
		return *this;
	}
	FVector& operator/=(float Scalar)
	{
		*this = *this / Scalar;
		return *this;
	}

	UnderlayingType Dot(const FVector& Other) const
	{
		return (X * Other.X) + (Y * Other.Y) + (Z * Other.Z);
	}
	UnderlayingType GetDistanceTo(const FVector& Other) const
	{
		FVector DiffVector = Other - *this;
		return DiffVector.Magnitude();
	}
	UnderlayingType GetDistanceToInMeters(const FVector& Other) const
	{
		return GetDistanceTo(Other) * 0.01;
	}
	FVector GetNormalized() const
	{
		return *this / Magnitude();
	}
	bool IsZero() const
	{
		return X == 0.0 && Y == 0.0 && Z == 0.0;
	}
	UnderlayingType Magnitude() const
	{
		return std::sqrt((X * X) + (Y * Y) + (Z * Z));
	}
	bool operator!=(const FVector& Other) const
	{
		return X != Other.X || Y != Other.Y || Z != Other.Z;
	}
	FVector operator*(const FVector& Other) const
	{
		return { X * Other.X, Y * Other.Y, Z * Other.Z };
	}
	FVector operator*(float Scalar) const
	{
		return { X * Scalar, Y * Scalar, Z * Scalar };
	}
	FVector operator+(const FVector& Other) const
	{
		return { X + Other.X, Y + Other.Y, Z + Other.Z };
	}
	FVector operator-(const FVector& Other) const
	{
		return { X - Other.X, Y - Other.Y, Z - Other.Z };
	}
	FVector operator/(const FVector& Other) const
	{
		if (Other.X == 0.0f || Other.Y == 0.0f || Other.Z == 0.0f)
			return *this;

		return { X / Other.X, Y / Other.Y, Z / Other.Z };
	}
	FVector operator/(float Scalar) const
	{
		if (Scalar == 0.0f)
			return *this;

		return { X / Scalar, Y / Scalar, Z / Scalar };
	}
	bool operator==(const FVector& Other) const
	{
		return X == Other.X && Y == Other.Y && Z == Other.Z;
	}
};

struct alignas(0x10) FVector4 final
{
public:
	float                                         X;                                                 // 0x0000(0x0004)(Edit, BlueprintVisible, ZeroConstructor, SaveGame, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	float                                         Y;                                                 // 0x0004(0x0004)(Edit, BlueprintVisible, ZeroConstructor, SaveGame, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	float                                         Z;                                                 // 0x0008(0x0004)(Edit, BlueprintVisible, ZeroConstructor, SaveGame, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	float                                         W;                                                 // 0x000C(0x0004)(Edit, BlueprintVisible, ZeroConstructor, SaveGame, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
};

struct FVector2D final
{
public:
	float                                         X;                                                 // 0x0000(0x0004)(Edit, BlueprintVisible, ZeroConstructor, SaveGame, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	float                                         Y;                                                 // 0x0004(0x0004)(Edit, BlueprintVisible, ZeroConstructor, SaveGame, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
};

struct FRotator final
{
public:
	float                                         Pitch;                                             // 0x0000(0x0004)(Edit, BlueprintVisible, ZeroConstructor, SaveGame, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	float                                         Yaw;                                               // 0x0004(0x0004)(Edit, BlueprintVisible, ZeroConstructor, SaveGame, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	float                                         Roll;                                              // 0x0008(0x0004)(Edit, BlueprintVisible, ZeroConstructor, SaveGame, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
};

struct alignas(0x10) FQuat final
{
public:
	float                                         X;                                                 // 0x0000(0x0004)(Edit, BlueprintVisible, ZeroConstructor, SaveGame, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	float                                         Y;                                                 // 0x0004(0x0004)(Edit, BlueprintVisible, ZeroConstructor, SaveGame, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	float                                         Z;                                                 // 0x0008(0x0004)(Edit, BlueprintVisible, ZeroConstructor, SaveGame, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
	float                                         W;                                                 // 0x000C(0x0004)(Edit, BlueprintVisible, ZeroConstructor, SaveGame, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
};