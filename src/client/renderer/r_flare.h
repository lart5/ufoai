#ifndef R_FLARE_H_
#define R_FLARE_H_

void R_CreateSurfaceFlare(mBspSurface_t *surf);
void R_DrawFlareSurfaces(const mBspSurfaces_t *surfs, glElementIndex_t *indexPtr);

#endif /* R_FLARE_H_ */
