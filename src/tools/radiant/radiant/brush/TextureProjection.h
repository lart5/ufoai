#ifndef TEXTUREPROJECTION_H_
#define TEXTUREPROJECTION_H_

#include "itextstream.h"
#include "TexDef.h"

class Winding;

/* greebo: A texture projection contains the texture definition
 */
class TextureProjection
{
	private:
		void basisForNormal (const Vector3& normal, Matrix4& basis);

	public:
		TexDef m_texdef;

		// Constructor
		TextureProjection ();

		// Copy Constructor
		TextureProjection (const TexDef& texdef);

		void constructDefault ();

		/* greebo: Uses the transformation matrix <transform> to set the internal texture
		 * definitions. Checks the matrix for validity and passes it on to
		 * the according internal texture definitions (TexDef or BPTexDef)
		 */
		void setTransform (float width, float height, const Matrix4& transform);

		/* greebo: Returns the transformation matrix from the
		 * texture definitions members.
		 */
		Matrix4 getTransform (float width, float height) const;

		// Normalise projection for a given texture width and height.
		void normalise (float width, float height);

		void fitTexture (std::size_t width, std::size_t height, const Vector3& normal, const Winding& w,
				float s_repeat, float t_repeat);

		/** greebo: Mirrors the texture around the given axis.
		 *
		 * @parameter: If the first component (x-component) of the vector is not 0, the texture is flipped
		 * around the x-axis. Same goes for the y-component (a value unequal to 0 flips it around y).
		 * The z-component is ignored.
		 *
		 * @flipAxis: Pass <1,0,0> to flipX, <0,1,0> to flipY (haven't tested what happens, if <1,1,0> is passed.
		 */
		void flipTexture(const Vector3& flipAxis);

		void transformLocked (std::size_t width, std::size_t height, const Plane3& plane,
				const Matrix4& identity2transformed);

		void emitTextureCoordinates (std::size_t width, std::size_t height, Winding& w, const Vector3& normal,
				const Matrix4& localToWorld);
};

#endif /*TEXTUREPROJECTION_H_*/
