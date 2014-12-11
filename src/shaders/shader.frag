#version 400

#define M_PI 3.14159265359
#define NUM_OBJS 7
#define NUM_LIGHTS 2
#define BLACK vec4(0.0, 0.0, 0.0, 1.0);
#define kA 0.5
#define kD 0.5
#define kS 0.5
#define MAXDEPTH 4
#define DELTA -1.0 / 512.0

out vec4 outColor;
uniform float width;
uniform float height;
uniform mat4 filmToWorld;
uniform mat4 inverseView;
uniform vec3 eye;
uniform float time;
uniform sampler2D textureSampler;
uniform sampler2D bg;

/*
 * 0 - Beauty
 * 1 - AO
 * 2 - Normals
 * 3 - Depth
 * 4 - Ambient
 * 5 - Diffuse
 * 6 - Spec
 */
// Settings Uniforms
uniform int renderPass;
uniform int shadows;
uniform int textureMapping;
uniform int reflections;
uniform int ambientOcclusion;
uniform int bump;
uniform int dof;

// Make this a uniform with a control
float bumpDepth = 5.0;

struct obj
{
    vec3 ca;
    vec3 cd;
    vec3 cs;
    vec3 cr;
    mat4 xform;
    float blend;
    int type;
    float shininess;
    vec3 pos;
    float radius;
} objs[NUM_OBJS];

struct light
{
    vec3 color;
    vec3 function;
    vec3 pos;
} lights[NUM_LIGHTS];

struct isect
{
    float t;
    vec3 pos;
    vec3 norm;
    obj obj;
    vec2 tex;
} i;

// RETURN SMALLER OF TWO NUMBERS THAT IS GREATER THAN 0, ELSE -1
float smallestPos(float a, float b)
{
    if (a <= 0.0 && b < 0.0) {
        return -1.0;
    }
    else if (a <= 0.0 && b > 0.0) {
        return b;
    }
    else if (a > 0.0 && b <= 0.0) {
        return a;
    }
    else {
        return min(a, b);
    }
}

// COMPUTE QUADRATIC FORMULA
float quadratic(float a, float b, float c)
{
    float dscr = pow(b, 2.0) - (4.0 * a * c);

    if (dscr < 0.0) {
        return -1.0;
    }
    else {
        return smallestPos((-b - pow(dscr, 0.5)) / (2.0 * a), (-b + pow(dscr, 0.5)) / (2.0 * a));
    }
}

// INTERSECT WITH OBJECT SPACE SPHERE
float intersectSphere(vec3 ro, vec3 rd)
{
    float a = pow(rd.x, 2.0) + pow(rd.y, 2.0) + pow(rd.z, 2.0);
    float b = 2.0 * (rd.x * ro.x + rd.y * ro.y + rd.z * ro.z);
    float c = pow(ro.x, 2.0) + pow(ro.y, 2.0) + pow(ro.z, 2.0) - 0.25;

    return quadratic(a, b, c);
}

// SAMPLE TEXTURE MAP FOR HEIGHT
float getHeight(vec2 uv, float blend)
{
    return ((1.0 - blend) * 1.0 + blend * texture2D(textureSampler, uv).r);
}

isect intersectObjs(vec3 ro, vec3 rd)
{
    i.t = -1.0;
    float minT = -1.0;
    obj minObj;
    float currT;
    vec3 minPos;
    for (int i = 0; i < NUM_OBJS; i++)
    {
        // Don't take inverses.  Precompute.
        vec3 p = vec3(inverse(objs[i].xform) * vec4(ro, 1.0));
        vec3 d = vec3(inverse(objs[i].xform) * vec4(rd, 0.0));

        currT = intersectSphere(p, d);

        if ((currT < minT && currT != -1.0) || (currT > minT && minT == -1.0))
        {
            minT = currT;
            minObj = objs[i];
            minPos = p + d * currT;
        }
    }
    i.t = minT;
    i.obj = minObj;
    i.pos = minPos;
    i.tex = vec2(0.5 - (atan(minPos.z, minPos.x) / (2.0 * M_PI)), 0.5 - (asin(minPos.y / sqrt(dot(minPos, minPos))) / M_PI));
    i.norm = normalize(minPos);
    
    // BUMP MAPPING
    if (bump == 1)
    {
        vec3 u = normalize(cross(i.norm, vec3(0.0, 1.0, 0.0)));
        vec3 v = normalize(cross(u, i.norm));
        mat3 m = mat3(u, v, i.norm);
        float A = texture2D(textureSampler, i.tex).r * bumpDepth;
        float B = texture2D(textureSampler, i.tex + vec2(DELTA, 0.0)).r * bumpDepth;
        float C = texture2D(textureSampler, i.tex + vec2(0.0, DELTA)).r * bumpDepth;
        vec3 norm = normalize(vec3(B - A, C - A, 0.25));
        i.norm = normalize(m * norm);
    }


    return i;
}

void init()
{
    objs[0].ca = vec3(0.0);
    objs[0].cd = vec3(1.0);
    objs[0].cs = vec3(1.0, 1.0, 1.0);
    objs[0].cr = vec3(0.3);
    objs[0].blend = 1.0;
    objs[0].xform = mat4(6.0 * cos(radians(time)), 0.0, 6.0 * -sin(radians(time)), 0.0,
                         0.0, 6.0, 0.0, 0.0,
                         6.0 * sin(radians(time)), 0.0, 6.0 * cos(radians(time)), 0.0,
                         0.0, 0.0, 0.0, 1.0);
    objs[0].pos = vec3(0.0, 0.0, 0.0);
    objs[0].radius = 3.0;
    objs[0].type = 0;
    objs[0].shininess = 50.0;

    objs[1].ca = vec3(0.0, 0.0, 0.0);
    objs[1].cd = vec3(1.0);
    objs[1].cs = vec3(1.0, 1.0, 1.0);
    objs[1].cr = vec3(0.5, 0.5, 0.5);
    objs[1].blend = 0.0;
    objs[1].xform = mat4(3.0, 0.0, 0.0, 0.0,
                         0.0, 3.0, 0.0, 0.0,
                         0.0, 0.0,  3.0, 0.0,
                         4.5, 3.0 * sin(time / 50.0),  0.0, 1.0);
    objs[1].pos = vec3(4.5, 3.0 * sin(time / 50.0), 0.0);
    objs[1].radius = 1.5;
    objs[1].type = 1;
    objs[1].shininess = 50.0;

    objs[2].ca = vec3(0.0, 0.0, 0.0);
    objs[2].cd = vec3(1.0);
    objs[2].cs = vec3(1.0, 1.0, 1.0);
    objs[2].cr = vec3(0.5, 0.5, 0.5);
    objs[2].blend = 0.0;
    objs[2].xform = mat4(3.0, 0.0, 0.0, 0.0,
                         0.0, 3.0, 0.0, 0.0,
                         0.0, 0.0,  3.0, 0.0,
                         -4.5, -3.0 * sin(time / 50.0),  0.0, 1.0);
    objs[2].pos = vec3(-4.5, -3.0 * sin(time / 50.0),  0.0);
    objs[2].radius = 1.5;
    objs[2].type = 2;
    objs[2].shininess = 50.0;

    objs[3].ca = vec3(0.0, 0.0, 0.0);
    objs[3].cd = vec3(1.0);
    objs[3].cs = vec3(1.0, 1.0, 1.0);
    objs[3].cr = vec3(0.5, 0.5, 0.5);
    objs[3].blend = 0.0;
    objs[3].xform = mat4(3.0, 0.0, 0.0, 0.0,
                         0.0, 3.0, 0.0, 0.0,
                         0.0, 0.0,  3.0, 0.0,
                         3.0 * cos(time / 50.0), 4.5,  0.0, 1.0);
    objs[3].pos = vec3(3.0 * cos(time / 50.0), 4.5,  0.0);
    objs[3].radius = 1.5;
    objs[3].type = 3;
    objs[3].shininess = 50.0;

    objs[4].ca = vec3(0.0, 0.0, 0.0);
    objs[4].cd = vec3(1.0);
    objs[4].cs = vec3(1.0, 1.0, 1.0);
    objs[4].cr = vec3(0.5, 0.5, 0.5);
    objs[4].blend = 0.0;
    objs[4].xform = mat4(3.0, 0.0, 0.0, 0.0,
                         0.0, 3.0, 0.0, 0.0,
                         0.0, 0.0,  3.0, 0.0,
                         -3.0 * cos(time / 50.0), -4.5,  0.0, 1.0);
    objs[4].pos = vec3(-3.0 * cos(time / 50.0), -4.5, 0.0);
    objs[4].radius = 1.5;
    objs[4].type = 4;
    objs[4].shininess = 50.0;

    objs[5].ca = vec3(0.2, 0.1, 0.15);
    objs[5].cd = vec3(1.0);
    objs[5].cs = vec3(1.0, 1.0, 1.0);
    objs[5].cr = vec3(0.5, 0.5, 0.5);
    objs[5].blend = 0.0;
    objs[5].xform = mat4(3.0, 0.0, 0.0, 0.0,
                         0.0, 3.0, 0.0, 0.0,
                         0.0, 0.0,  3.0, 0.0,
                         0.0, 3.0 * cos(time / 50.0),  4.5, 1.0);
    objs[5].pos = vec3(0.0, 3.0 * cos(time / 50.0), 4.5);
    objs[5].radius = 1.5;
    objs[5].type = 5;
    objs[5].shininess = 50.0;

    objs[6].ca = vec3(0.0, 0.0, 0.0);
    objs[6].cd = vec3(1.0);
    objs[6].cs = vec3(1.0, 1.0, 1.0);
    objs[6].cr = vec3(0.5, 0.5, 0.5);
    objs[6].blend = 0.0;
    objs[6].xform = mat4(3.0, 0.0, 0.0, 0.0,
                         0.0, 3.0, 0.0, 0.0,
                         0.0, 0.0,  3.0, 0.0,
                         0.0, -3.0 * cos(time / 50.0),  -4.5, 1.0);
    objs[6].pos = vec3(0.0, -3.0 * cos(time / 50.0), -4.5);
    objs[6].radius = 1.5;
    objs[6].type = 6;
    objs[6].shininess = 50.0;

    lights[0].color = vec3(1.0, 1.0, 1.0);
    lights[0].function = vec3(0.0, 0.0, 0.0);
    lights[0].pos = vec3(1.0, -1.8, -2.0);

    lights[1].color = vec3(1.0, 1.0, 1.0);
    lights[1].function = vec3(0.0, 0.0, 0.0);
    lights[1].pos = vec3(0.25, 1.0, -1.0);
}

vec3 calculateLighting(vec3 pos, vec3 rd, isect o)
{
    vec3 res = vec3(0.0);
    if (renderPass != 5 && renderPass != 6) {
        res += o.obj.ca;
    }

    if (renderPass != 4) {
        for (int j = 0; j < NUM_LIGHTS; j++)
        {
            vec3 lightDir = normalize(-lights[j].pos);
            vec3 reflectionVec = normalize(reflect(lightDir, o.norm));
            vec3 posAug = pos + (o.norm / 1000.0);

            vec3 diffuseColor = vec3(0.0);
            if (textureMapping == 1) {
                diffuseColor = ((1.0 - o.obj.blend) * o.obj.cd + o.obj.blend * texture2D(textureSampler, o.tex).rgb);
            }
            else {
                diffuseColor = o.obj.cd;
            }

            float falloff = 1.0;
            if (renderPass != 5) {
                res.r += min(1.0, pow(max(0.0, dot(reflectionVec, rd)), o.obj.shininess) * lights[j].color.r * o.obj.cs.r * kS) / falloff;
                res.g += min(1.0, pow(max(0.0, dot(reflectionVec, rd)), o.obj.shininess) * lights[j].color.g * o.obj.cs.g * kS) / falloff;
                res.b += min(1.0, pow(max(0.0, dot(reflectionVec, rd)), o.obj.shininess) * lights[j].color.b * o.obj.cs.b * kS) / falloff;
            }

            if (renderPass != 6) {
                res.r += max(0.0, dot(o.norm, lightDir) * lights[j].color.r * diffuseColor.r * kD) / falloff;
                res.g += max(0.0, dot(o.norm, lightDir) * lights[j].color.g * diffuseColor.g * kD) / falloff;
                res.b += max(0.0, dot(o.norm, lightDir) * lights[j].color.b * diffuseColor.b * kD) / falloff;
            }
        }
    }

    return res;
}


void main(void)
{

    // COMPUTE RAY ORIGIN AND DIRECTION
    float x = gl_FragCoord.x;
    float y = height - gl_FragCoord.y;

    vec4 film = vec4(((2.0 * x) / width) - 1.0, 1.0 - ((2.0 * y) / height), -1.0, 1.0);
    vec4 world = filmToWorld * film;
    vec3 rd = normalize(world.xyz / world.w - eye);
    vec3 ro = eye;

    // INITIALIZE SCENE GEOMETRY
    init();

    vec3 res = vec3(0.0);
    vec3 spec = vec3(1.0);
    int depth = MAXDEPTH;

    // INTERSECT SCENE
    i = intersectObjs(ro, rd);
    float firstT = i.t;
    if (firstT == -1.0) {
        firstT = 100.0;
    }

    // COMPUTE WORLD POSITION AND WORLD NORMAL
    vec3 worldPos = rd * i.t + ro;
    i.norm = normalize(inverse(transpose(mat3(i.obj.xform))) * i.norm);

    // DEPTH PASS
    if (renderPass == 3) {

        if (i.t == -1.0) {
            outColor = vec4(1.0);
            return;
        }
        outColor = vec4(i.t / 100.0, i.t / 100.0, i.t / 100.0, 1.0);
        return;
    }

    // BACKGROUND COLOR ON MISS
    if (i.t == -1.0) {
        return;
        
    }

    // NORMAL PASS
    if (renderPass == 2)
    {
        outColor = vec4(i.norm, 1.0);
        return;
    }

    // AMBIENT OCCLUSION
    float ao = 1.0;
    if (ambientOcclusion == 1 || renderPass == 1) {
        ao = 0.0;
        for (int j = 0; j < NUM_OBJS; j++)
        {
            if (j == i.obj.type) {
                continue;
            }
            vec3 dir = objs[j].pos - worldPos;
            float len = length(dir);
            float normLen = dot(i.norm, dir / len);
            float h = len / objs[j].radius;
            float h2 = h * h;
            ao += normLen / h2;
        }
        ao = 1.0 - ao;
    }

    // AMBIENT OCCLUSION PASS
    if (renderPass == 1) 
    {
        outColor = vec4(ao);
        return;
    }

    // COMPUTE LIGHTING FOR INTERSECTION AND FIND REFLECTED RAY
    res += spec * calculateLighting(worldPos, rd, i);
    spec *= i.obj.cr;

    rd = reflect(rd, i.norm);
    ro = worldPos + (rd / 1000.0);

    // NO RECURSION ON AMBIENT, DIFFUSE, OR SPEC PASSES
    if (reflections != 1 || renderPass == 4 || renderPass == 5 || renderPass == 6) {
        depth = 1;
    }

    // 'RECURSIVE' LOOP
    for (int j = 0; j < depth - 1; j++)
    {
        i = intersectObjs(ro, rd);

        if (i.t == -1.0) {
            break;
        }

        vec3 worldPos = rd * i.t + ro;
        i.norm = normalize(inverse(transpose(mat3(i.obj.xform))) * i.norm);
        res += spec * calculateLighting(worldPos, rd, i);
        spec *= i.obj.cr;

        rd = reflect(rd, i.norm);
        ro = worldPos + (rd / 1000.0);
    }

    // CLAMP COLOR AND STORE DEPTH IN ALPHA CHANNEL
    outColor = vec4(clamp(res, vec3(0.0), vec3(1.0)), clamp(1.0 / sqrt(firstT), 0.0, 1.0));

    // MULTIPLY IN AO
    if (ambientOcclusion == 1) {
        outColor = outColor * ao;
    }
}

