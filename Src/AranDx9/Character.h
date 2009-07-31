#pragma once

//#include <d3dx9.h>

#include "CharacterInterface.h"

class CharacterAnimationCallback;
class ModelReader;

namespace Aran
{
	class Character:
		public CharacterInterface
	{
	public:
		Character(void);
		Character(ArnVec3 translation, ArnVec3 scale, ArnQuat rotation);
		~Character(void);

		void Initialize();

		virtual void ChangeTranslationToLookAtDirection( float amount );
		virtual void ChangeTranslation( float dx, float dy, float dz );
		virtual void ChangeOrientation( float dx, float dy, float dz ) /* radian */;
		virtual const ArnMatrix* GetFinalTransform() const;


		virtual void SetCharacterAnimationState(CharacterAnimationState cas);
		virtual void SetCharacterAnimationStateNext(CharacterAnimationState cas);

		virtual CharacterAnimationState GetCharacterAnimationState() { return this->animState; }
		virtual CharacterAnimationState GetCharacterAnimationStateNext() { return this->animStateNext; }

		HRESULT RegisterCharacterAnimationCallback( CharacterAnimationState cas, CharacterAnimationCallback* pCAC );
		HRESULT UnregisterCharacterAnimationCallback( CharacterAnimationState cas );

		HRESULT AttachModelReader( const ModelReader* pMR );
		const ModelReader* GetModelReader() const;
		float GetAnimStateWeight() const { return this->animStateWeight; }
		void SetAnimStateWeight( float f )
		{
			if ( f < 0.0f )
				f = 0.0f;

			this->animStateWeight = f;
		}

	private:
		ArnMatrix finalTransform;
		ArnVec3 translation, scale;
		ArnQuat rotation;

		ArnVec3 lookAt; // character's eye(font) direction
		ArnVec4 outLookAt;


		CharacterAnimationCallback* callbacks[CAS_SIZE];

		//           current weight            next weight
		//                1.0f                     0.0f
		// TRANSITION!--->  --->  --->
		//                0.9f                     0.1f
		//                ...                      ...
		//                0.1f                     0.9f
		//                0.0f                     1.0f;
		// STATE SWAP(SHIFT)! ---> ---> --->
		//                1.0f(next state)         0.0f(current state or whatever);
		//
		CharacterAnimationState animState;		// current anim state
		float animStateWeight;					// current anim state weight
		CharacterAnimationState animStateNext;	// next anim state


		const ModelReader* pMR; // attached model reader

	};
}


