#pragma once 
void AimAt(Vector2 ClosestPos)
{
	float TargetX = 0;
	float TargetY = 0;

	if (ClosestPos.x != 0) {
		if (ClosestPos.x > ScreenCenter.x) {
			TargetX = -(ScreenCenter.x - ClosestPos.x);
			if (TargetX + ScreenCenter.x > ScreenCenter.x * 2)
				TargetX = 0;
		}

		if (ClosestPos.x < ScreenCenter.x) {
			TargetX = ClosestPos.x - ScreenCenter.x;
			if (TargetX + ScreenCenter.x < 0)
				TargetX = 0;
		}
	}

	if (ClosestPos.y != 0) {
		if (ClosestPos.y > ScreenCenter.y) {
			TargetY = -(ScreenCenter.y - ClosestPos.y);
			if (TargetY + ScreenCenter.y > ScreenCenter.y * 2)
				TargetY = 0;
		}

		if (ClosestPos.y < ScreenCenter.y) {
			TargetY = ClosestPos.y - ScreenCenter.y;
			if (TargetY + ScreenCenter.y < 0)
				TargetY = 0;
		}
	}

	TargetX /= Smooth;
	TargetY /= Smooth;

	mouse_event(MOUSEEVENTF_MOVE, static_cast<int>(TargetX), static_cast<int>(TargetY), 0, 0);
}

void MemAim(Vector3 ClosesPos, std::uint64_t Camera)
{
	device_t.write<Vector3>(Camera + 0x40, Vector3());
	device_t.write<Vector3>(Camera + 0x3D0, Vector3());
}