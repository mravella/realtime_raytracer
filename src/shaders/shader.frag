#version 400


#define M_PI 3.14159265359
#define M_INF 1e16
#define NUM_OBJS 7
#define NUM_LIGHTS 2
#define BLACK vec4(0.0, 0.0, 0.0, 1.0);
#define kA 0.5
#define kD 0.5
#define kS 0.5
#define MAXDEPTH 2
#define DELTA -1.0 / 512.0

out vec4 outColor;
uniform float width;
uniform float height;
uniform mat4 filmToWorld;
uniform mat4 inverseView;
uniform vec3 eye;
uniform int settings;
uniform float time;
uniform sampler2D textureSampler;
uniform sampler2D bg;

// Make this a uniform with a control
float bumpDepth = 10.0;

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

float intersectSphere(vec3 ro, vec3 rd)
{
    float a = pow(rd.x, 2.0) + pow(rd.y, 2.0) + pow(rd.z, 2.0);
    float b = 2.0 * (rd.x * ro.x + rd.y * ro.y + rd.z * ro.z);
    float c = pow(ro.x, 2.0) + pow(ro.y, 2.0) + pow(ro.z, 2.0) - 0.25;

    return quadratic(a, b, c);
}

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
// 	vec3 u = normalize(cross(i.norm, vec3(0.0, 1.0, 0.0)));
// 	vec3 v = normalize(cross(u, i.norm));
// 	mat3 m = mat3(u, v, i.norm);
// 	float A = texture2D(textureSampler, i.tex).r * bumpDepth;
// 	float B = texture2D(textureSampler, i.tex + vec2(DELTA, 0.0)).r * bumpDepth;
// 	float C = texture2D(textureSampler, i.tex + vec2(0.0, DELTA)).r * bumpDepth;
// 	vec3 norm = normalize(vec3(B - A, C - A, 0.25));
// 	i.norm = normalize(m * norm);


    return i;
}

void init()
{
    objs[0].ca = vec3(0.0, 0.0, 0.0);
    objs[0].cd = vec3(0.3);
    objs[0].cs = vec3(1.0, 1.0, 1.0);
    objs[0].cr = vec3(0.3);
    objs[0].blend = 0.5;
    objs[0].xform = mat4(6.0 * cos(radians(time)), 0.0, 6.0 * -sin(radians(time)), 0.0,
                         0.0, 6.0, 0.0, 0.0,
                         6.0 * sin(radians(time)), 0.0, 6.0 * cos(radians(time)), 0.0,
                         0.0, 0.0, 0.0, 1.0);
    objs[1].pos = vec3(4.5, 3.0 * sin(time / 50.0), 0.0);
    objs[1].radius = 3.0;
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
    objs[1].radius = 3.0;
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
    objs[2].radius = 3.0;
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
    objs[3].radius = 3.0;
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
    objs[4].radius = 3.0;
    objs[4].type = 4;
    objs[4].shininess = 50.0;

    objs[5].ca = vec3(0.0, 0.0, 0.0);
    objs[5].cd = vec3(1.0);
    objs[5].cs = vec3(1.0, 1.0, 1.0);
    objs[5].cr = vec3(0.5, 0.5, 0.5);
    objs[5].blend = 0.0;
    objs[5].xform = mat4(3.0, 0.0, 0.0, 0.0,
                         0.0, 3.0, 0.0, 0.0,
                         0.0, 0.0,  3.0, 0.0,
                         0.0, 3.0 * cos(time / 50.0),  4.5, 1.0);
    objs[5].pos = vec3(0.0, 3.0 * cos(time / 50.0), 4.5);
    objs[5].radius = 3.0;
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
    objs[6].radius = 3.0;
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
    vec3 res = o.obj.ca;
    for (int j = 0; j < NUM_LIGHTS; j++)
    {
        // vec3 vertToLight = lights[i].pos - pos;
        // vec3 lightDir = normalize(vertToLight);
        vec3 lightDir = normalize(-lights[j].pos);
        // float dist = sqrt(vertToLight.x * vertToLight.x + vertToLight.y * vertToLight.y + vertToLight.z * vertToLight.z);

        vec3 reflectionVec = normalize(reflect(lightDir, o.norm));
        vec3 posAug = pos + (o.norm / 1000.0);

        // float falloff = max(1.0, (lights[i].function.x + dist * lights[i].function.y + dist * dist * lights[i].function.z));
        float falloff = 1.0;
        res.r += min(1.0, pow(max(0.0, dot(reflectionVec, rd)), o.obj.shininess) * lights[j].color.r * o.obj.cs.r * kS) / falloff;
        res.g += min(1.0, pow(max(0.0, dot(reflectionVec, rd)), o.obj.shininess) * lights[j].color.g * o.obj.cs.g * kS) / falloff;
        res.b += min(1.0, pow(max(0.0, dot(reflectionVec, rd)), o.obj.shininess) * lights[j].color.b * o.obj.cs.b * kS) / falloff;

        res.r += max(0.0, dot(o.norm, lightDir) * lights[j].color.r * ((1.0 - o.obj.blend) * o.obj.cd.r + o.obj.blend * texture2D(textureSampler, o.tex).r) * kD) / falloff;
        res.g += max(0.0, dot(o.norm, lightDir) * lights[j].color.g * ((1.0 - o.obj.blend) * o.obj.cd.g + o.obj.blend * texture2D(textureSampler, o.tex).g) * kD) / falloff;
        res.b += max(0.0, dot(o.norm, lightDir) * lights[j].color.b * ((1.0 - o.obj.blend) * o.obj.cd.b + o.obj.blend * texture2D(textureSampler, o.tex).b) * kD) / falloff;


    }
    return res;
}


void main(void)
{
    if (settings == 2) {
        outColor = vec4(184.0 / 255.0, 169.0 / 255.0, 204.0 / 255.0, 1.0);
        return;
    }

    float x = gl_FragCoord.x;
    float y = height - gl_FragCoord.y;

    vec4 film = vec4(((2.0 * x) / width) - 1.0, 1.0 - ((2.0 * y) / height), -1.0, 1.0);
    vec4 world = filmToWorld * film;
    vec3 rd = normalize(world.xyz / world.w - eye);
    vec3 ro = eye;

    init();

    vec3 res = vec3(0.0);
    vec3 spec = vec3(1.0);
    int depth = MAXDEPTH;

    i = intersectObjs(ro, rd);
    float firstT = i.t;
    if (firstT == -1.0) {
        firstT = 100.0;
    }

    // DEPTH BUFFER
    
    if (settings == 4) {

        if (i.t == -1.0) {
            outColor = vec4(1.0);
            return;
        }
        outColor = vec4(i.t / 100.0, i.t / 100.0, i.t / 100.0, 1.0);
        return;
    }

    if (i.t == -1.0) {
        return;
        
    }

    vec3 worldPos = rd * i.t + ro;

    // AMBIENT OCCLUSION

   float ao = 0.0;
   for (int j = 0; j < NUM_OBJS; j++)
   {
       if (j == i.obj.type) {
           continue;
       }
       vec3 dir = objs[j].pos - i.pos;
       float len = max(0.01, length(dir));
       float normLen = dot(i.norm, dir / len);
       float h = len / objs[j].radius;
       float h2 = max(0.01, h * h);
       ao += max(0.0, normLen) / h2;
   }

   outColor = vec4(1.0 - ao);
   return;

    // Fix Spec & maybe do less matrix operations for the speed
    i.norm = normalize(inverse(transpose(mat3(i.obj.xform))) * i.norm);
    res += spec * calculateLighting(worldPos, rd, i);
    spec *= i.obj.cr;

    rd = reflect(rd, i.norm);
    ro = worldPos + (rd / 1000.0);


    for (int j = 0; j < depth - 1; j++)
    {
        i = intersectObjs(ro, rd);

        if (i.t == -1.0) {
            break;
        }

        vec3 worldPos = rd * i.t + ro;
        // Fix Spec & maybe do less matrix operations for the speed
        i.norm = normalize(inverse(transpose(mat3(i.obj.xform))) * i.norm);
        res += spec * calculateLighting(worldPos, rd, i);
        spec *= i.obj.cr;

        rd = reflect(rd, i.norm);
        ro = worldPos + (rd / 1000.0);
    }

    outColor = vec4(clamp(res, vec3(0.0), vec3(1.0)), clamp(1.0 / sqrt(firstT), 0.0, 1.0));

    // outColor = vec4( clamp(1.0 / sqrt(firstT), 0.0, 1.0));
    // outColor = vec4(texture2D(textureSampler, vec2(x, y) / height).rgb, 1.0);
}

