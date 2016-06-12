#version 130
// Texturas
uniform sampler2D stexflat;
uniform sampler2D stexcentral;
uniform sampler2D stexrelleno01;
uniform sampler2D stexrelleno02;
uniform sampler2D stexpiso;

// Intensidad Luces
uniform float ambientalInt;
uniform float centralInt;
uniform float relleno01Int;
uniform float relleno02Int;

// Color Luces
uniform vec4 ambientalColor;
uniform vec4 centralColor;
uniform vec4 relleno01Color;
uniform vec4 relleno02Color;
uniform vec4 pisoColor;

// Filtro
uniform bool filtroBilineal;
uniform bool pisoAct;

// Algoritmo Filtro Bilineal
vec4 texture2D_bilinear( sampler2D tex, vec2 uv )
{
	vec2 f;

	ivec2 textureSize2d = textureSize(tex,0);
	float textureWidth = float(textureSize2d.x); 
	float textureHeight = float(textureSize2d.y);	
	float texel_size_x = 1.0 / textureWidth;
	float texel_size_y = 1.0 / textureHeight;

	f.x	= fract( uv.x * textureWidth );
	f.y	= fract( uv.y * textureHeight );

	vec4 t00 = texture2D( tex, uv + vec2( 0.0, 0.0 ));
	vec4 t10 = texture2D( tex, uv + vec2( texel_size_x, 0.0 ));
	vec4 tA = mix( t00, t10, f.x);

	vec4 t01 = texture2D( tex, uv + vec2( 0.0, texel_size_y ) );
	vec4 t11 = texture2D( tex, uv + vec2( texel_size_x, texel_size_y ) );
	vec4 tB = mix( t01, t11, f.x );

	return mix( tA, tB, f.y );
}

void main(void) {

	vec4 cFinal;
	vec4 cAmbiental;
	vec4 cCentral;
	vec4 cRelleno01;
	vec4 cRelleno02;
	vec4 cPiso;

	vec4 auxAmbiental;
	vec4 auxCentral;
	vec4 auxRelleno;
	vec4 auxPiso;
	vec4 aux;

	if (filtroBilineal) {
		cAmbiental = texture2D_bilinear(stexflat,gl_TexCoord[0].st);
		cCentral   = texture2D_bilinear(stexcentral,gl_TexCoord[0].st);
		cRelleno01 = texture2D_bilinear(stexrelleno01,gl_TexCoord[0].st);
		cRelleno02 = texture2D_bilinear(stexrelleno02,gl_TexCoord[0].st);
		cPiso   = texture2D_bilinear(stexpiso,gl_TexCoord[0].st);
	}else{
		cAmbiental = texture2D(stexflat,gl_TexCoord[0].st);
		cCentral   = texture2D(stexcentral,gl_TexCoord[0].st);
		cRelleno01 = texture2D(stexrelleno01,gl_TexCoord[0].st);
		cRelleno02 = texture2D(stexrelleno02,gl_TexCoord[0].st);
		cPiso   = texture2D(stexpiso,gl_TexCoord[0].st);
	}

	if (pisoAct){
		auxPiso = mix(cAmbiental, pisoColor,cPiso) ;
	} else {
		auxPiso = cAmbiental;
	}

	auxAmbiental = auxPiso * ambientalInt * ambientalColor;
	auxCentral   = cCentral * centralInt * centralColor;
	auxRelleno   = (cRelleno02 * relleno02Int * relleno02Color)
				 + (cRelleno01 * relleno01Int * relleno01Color);

	gl_FragColor = auxAmbiental * (auxCentral + auxRelleno);
}