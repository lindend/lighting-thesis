#include "CrazeEngine.h"
#include "Win32KeyMapper.h"
#include "InputManager.h"

#define KCM(winkey, crazekey) case VK_##winkey##: \
	return Craze::KC_##crazekey
#define KCS(key) KCM(key, key)

//The #@ operator is microsoft specific and puts ' ' around the key.
#define KCC(key) case #@key: \
	return Craze::KC_##key


Craze::KEYCODE Craze::Win32KeyMapper::ConvertKey(unsigned int key)
{
	switch(key)
	{
		KCS(ESCAPE);
		KCM(UP, UPARROW);
		KCM(DOWN, DOWNARROW);
		KCM(LEFT, LEFTARROW);
		KCM(RIGHT, RIGHTARROW);
		KCC(A);
		KCC(B);
		KCC(C);
		KCC(D);
		KCC(E);
		KCC(F);
		KCC(G);
		KCC(H);
		KCC(I);
		KCC(J);
		KCC(K);
		KCC(L);
		KCC(M);
		KCC(N);
		KCC(O);
		KCC(P);
		KCC(Q);
		KCC(R);
		KCC(S);
		KCC(T);
		KCC(U);
		KCC(V);
		KCC(W);
		KCC(X);
		KCC(Y);
		KCC(Z);
		KCS(F1);
		KCS(F2);
		KCS(F3);
		KCS(F4);
		KCS(F5);
		KCS(F6);
		KCS(F7);
		KCS(F8);
		KCS(F9);
		KCS(F10);
		KCS(F11);
		KCS(F12);

	};

	return Craze::KC_UNKNOWN;
}