#ifndef GLADIO_REQUEST_CODES_H
#define GLADIO_REQUEST_CODES_H

#define REQUEST_CODE_SET_CURRENT_RENDER_WINDOW 1
#define REQUEST_CODE_SWAP_DISPLAY_BUFFERS 2

#define REQUEST_CODE_GL_DSA_START 10
#define REQUEST_CODE_GL_DSA_SAVE_ACTIVE_TEXTURE 10
#define REQUEST_CODE_GL_DSA_RESTORE_ACTIVE_TEXTURE 11
#define REQUEST_CODE_GL_DSA_SAVE_BOUND_TEXTURE 12
#define REQUEST_CODE_GL_DSA_RESTORE_BOUND_TEXTURE 13
#define REQUEST_CODE_GL_DSA_SAVE_BOUND_BUFFER 14
#define REQUEST_CODE_GL_DSA_RESTORE_BOUND_BUFFER 15
#define REQUEST_CODE_GL_DSA_SAVE_BOUND_ARB_PROGRAM 16
#define REQUEST_CODE_GL_DSA_RESTORE_BOUND_ARB_PROGRAM 17
#define REQUEST_CODE_GL_DSA_SAVE_BOUND_FRAMEBUFFER 18
#define REQUEST_CODE_GL_DSA_RESTORE_BOUND_FRAMEBUFFER 19
#define REQUEST_CODE_GL_DSA_SAVE_BOUND_RENDERBUFFER 20
#define REQUEST_CODE_GL_DSA_RESTORE_BOUND_RENDERBUFFER 21

#define REQUEST_CODE_GL_CALL_START 100
#define REQUEST_CODE_GL_CALL_COUNT 430

#define REQUEST_CODE_GL_ACCUM 100
#define REQUEST_CODE_GL_ACTIVE_TEXTURE 101
#define REQUEST_CODE_GL_ALPHA_FUNC 102
#define REQUEST_CODE_GL_ARRAY_ELEMENT 103
#define REQUEST_CODE_GL_ATTACH_SHADER 104
#define REQUEST_CODE_GL_BEGIN 105
#define REQUEST_CODE_GL_BEGIN_CONDITIONAL_RENDER 106
#define REQUEST_CODE_GL_BEGIN_QUERY 107
#define REQUEST_CODE_GL_BEGIN_TRANSFORM_FEEDBACK 108
#define REQUEST_CODE_GL_BIND_ATTRIB_LOCATION 109
#define REQUEST_CODE_GL_BIND_BUFFER 110
#define REQUEST_CODE_GL_BIND_BUFFER_BASE 111
#define REQUEST_CODE_GL_BIND_BUFFER_RANGE 112
#define REQUEST_CODE_GL_BIND_FRAG_DATA_LOCATION 113
#define REQUEST_CODE_GL_BIND_FRAG_DATA_LOCATION_INDEXED 114
#define REQUEST_CODE_GL_BIND_FRAMEBUFFER 115
#define REQUEST_CODE_GL_BIND_PROGRAM_ARB 116
#define REQUEST_CODE_GL_BIND_RENDERBUFFER 117
#define REQUEST_CODE_GL_BIND_SAMPLER 118
#define REQUEST_CODE_GL_BIND_TEXTURE 119
#define REQUEST_CODE_GL_BIND_VERTEX_ARRAY 120
#define REQUEST_CODE_GL_BITMAP 121
#define REQUEST_CODE_GL_BLEND_COLOR 122
#define REQUEST_CODE_GL_BLEND_EQUATION 123
#define REQUEST_CODE_GL_BLEND_EQUATION_SEPARATE 124
#define REQUEST_CODE_GL_BLEND_EQUATION_SEPARATEI 125
#define REQUEST_CODE_GL_BLEND_EQUATIONI 126
#define REQUEST_CODE_GL_BLEND_FUNC 127
#define REQUEST_CODE_GL_BLEND_FUNC_SEPARATE 128
#define REQUEST_CODE_GL_BLEND_FUNC_SEPARATEI 129
#define REQUEST_CODE_GL_BLEND_FUNCI 130
#define REQUEST_CODE_GL_BLIT_FRAMEBUFFER 131
#define REQUEST_CODE_GL_BUFFER_DATA 132
#define REQUEST_CODE_GL_BUFFER_STORAGE 133
#define REQUEST_CODE_GL_BUFFER_SUB_DATA 134
#define REQUEST_CODE_GL_CALL_LIST 135
#define REQUEST_CODE_GL_CALL_LISTS 136
#define REQUEST_CODE_GL_CHECK_FRAMEBUFFER_STATUS 137
#define REQUEST_CODE_GL_CLAMP_COLOR 138
#define REQUEST_CODE_GL_CLEAR 139
#define REQUEST_CODE_GL_CLEAR_ACCUM 140
#define REQUEST_CODE_GL_CLEAR_BUFFERFI 141
#define REQUEST_CODE_GL_CLEAR_BUFFERFV 142
#define REQUEST_CODE_GL_CLEAR_BUFFERIV 143
#define REQUEST_CODE_GL_CLEAR_BUFFERUIV 144
#define REQUEST_CODE_GL_CLEAR_COLOR 145
#define REQUEST_CODE_GL_CLEAR_DEPTHF 146
#define REQUEST_CODE_GL_CLEAR_INDEX 147
#define REQUEST_CODE_GL_CLEAR_STENCIL 148
#define REQUEST_CODE_GL_CLIENT_ACTIVE_TEXTURE 149
#define REQUEST_CODE_GL_CLIENT_WAIT_SYNC 150
#define REQUEST_CODE_GL_CLIP_PLANE 151
#define REQUEST_CODE_GL_COLOR4F 152
#define REQUEST_CODE_GL_COLOR_MASK 153
#define REQUEST_CODE_GL_COLOR_MASKI 154
#define REQUEST_CODE_GL_COLOR_MATERIAL 155
#define REQUEST_CODE_GL_COLOR_POINTER 156
#define REQUEST_CODE_GL_COMPILE_SHADER 157
#define REQUEST_CODE_GL_COMPRESSED_TEX_IMAGE2D 158
#define REQUEST_CODE_GL_COMPRESSED_TEX_SUB_IMAGE2D 159
#define REQUEST_CODE_GL_COPY_BUFFER_SUB_DATA 160
#define REQUEST_CODE_GL_COPY_PIXELS 161
#define REQUEST_CODE_GL_COPY_TEX_IMAGE2D 162
#define REQUEST_CODE_GL_COPY_TEX_SUB_IMAGE2D 163
#define REQUEST_CODE_GL_COPY_TEX_SUB_IMAGE3D 164
#define REQUEST_CODE_GL_CREATE_PROGRAM 165
#define REQUEST_CODE_GL_CREATE_SHADER 166
#define REQUEST_CODE_GL_CULL_FACE 167
#define REQUEST_CODE_GL_DELETE_BUFFERS 168
#define REQUEST_CODE_GL_DELETE_FRAMEBUFFERS 169
#define REQUEST_CODE_GL_DELETE_LISTS 170
#define REQUEST_CODE_GL_DELETE_OBJECT_ARB 171
#define REQUEST_CODE_GL_DELETE_PROGRAM 172
#define REQUEST_CODE_GL_DELETE_PROGRAMS_ARB 173
#define REQUEST_CODE_GL_DELETE_QUERIES 174
#define REQUEST_CODE_GL_DELETE_RENDERBUFFERS 175
#define REQUEST_CODE_GL_DELETE_SAMPLERS 176
#define REQUEST_CODE_GL_DELETE_SHADER 177
#define REQUEST_CODE_GL_DELETE_SYNC 178
#define REQUEST_CODE_GL_DELETE_TEXTURES 179
#define REQUEST_CODE_GL_DELETE_VERTEX_ARRAYS 180
#define REQUEST_CODE_GL_DEPTH_FUNC 181
#define REQUEST_CODE_GL_DEPTH_MASK 182
#define REQUEST_CODE_GL_DEPTH_RANGEF 183
#define REQUEST_CODE_GL_DETACH_SHADER 184
#define REQUEST_CODE_GL_DISABLE 185
#define REQUEST_CODE_GL_DISABLE_CLIENT_STATE 186
#define REQUEST_CODE_GL_DISABLE_VERTEX_ATTRIB_ARRAY 187
#define REQUEST_CODE_GL_DISABLEI 188
#define REQUEST_CODE_GL_DISPATCH_COMPUTE 189
#define REQUEST_CODE_GL_DISPATCH_COMPUTE_INDIRECT 190
#define REQUEST_CODE_GL_DRAW_ARRAYS 191
#define REQUEST_CODE_GL_DRAW_ARRAYS_INSTANCED 192
#define REQUEST_CODE_GL_DRAW_BUFFER 193
#define REQUEST_CODE_GL_DRAW_BUFFERS 194
#define REQUEST_CODE_GL_DRAW_ELEMENTS 195
#define REQUEST_CODE_GL_DRAW_ELEMENTS_BASE_VERTEX 196
#define REQUEST_CODE_GL_DRAW_ELEMENTS_INSTANCED 197
#define REQUEST_CODE_GL_DRAW_ELEMENTS_INSTANCED_BASE_VERTEX 198
#define REQUEST_CODE_GL_DRAW_PIXELS 199
#define REQUEST_CODE_GL_DRAW_RANGE_ELEMENTS 200
#define REQUEST_CODE_GL_DRAW_RANGE_ELEMENTS_BASE_VERTEX 201
#define REQUEST_CODE_GL_EDGE_FLAG 202
#define REQUEST_CODE_GL_EDGE_FLAG_POINTER 203
#define REQUEST_CODE_GL_EDGE_FLAGV 204
#define REQUEST_CODE_GL_ENABLE 205
#define REQUEST_CODE_GL_ENABLE_CLIENT_STATE 206
#define REQUEST_CODE_GL_ENABLE_VERTEX_ATTRIB_ARRAY 207
#define REQUEST_CODE_GL_ENABLEI 208
#define REQUEST_CODE_GL_END 209
#define REQUEST_CODE_GL_END_CONDITIONAL_RENDER 210
#define REQUEST_CODE_GL_END_LIST 211
#define REQUEST_CODE_GL_END_QUERY 212
#define REQUEST_CODE_GL_END_TRANSFORM_FEEDBACK 213
#define REQUEST_CODE_GL_EVAL_COORD2F 214
#define REQUEST_CODE_GL_EVAL_MESH1 215
#define REQUEST_CODE_GL_EVAL_MESH2 216
#define REQUEST_CODE_GL_EVAL_POINT1 217
#define REQUEST_CODE_GL_EVAL_POINT2 218
#define REQUEST_CODE_GL_FEEDBACK_BUFFER 219
#define REQUEST_CODE_GL_FENCE_SYNC 220
#define REQUEST_CODE_GL_FINISH 221
#define REQUEST_CODE_GL_FLUSH 222
#define REQUEST_CODE_GL_FLUSH_MAPPED_BUFFER_RANGE 223
#define REQUEST_CODE_GL_FOG_COORD_POINTER 224
#define REQUEST_CODE_GL_FOG_COORDF 225
#define REQUEST_CODE_GL_FOGF 226
#define REQUEST_CODE_GL_FOGFV 227
#define REQUEST_CODE_GL_FRAMEBUFFER_RENDERBUFFER 228
#define REQUEST_CODE_GL_FRAMEBUFFER_TEXTURE 229
#define REQUEST_CODE_GL_FRAMEBUFFER_TEXTURE2D 230
#define REQUEST_CODE_GL_FRAMEBUFFER_TEXTURE3D 231
#define REQUEST_CODE_GL_FRAMEBUFFER_TEXTURE_LAYER 232
#define REQUEST_CODE_GL_FRONT_FACE 233
#define REQUEST_CODE_GL_FRUSTUM 234
#define REQUEST_CODE_GL_GEN_BUFFERS 235
#define REQUEST_CODE_GL_GEN_FRAMEBUFFERS 236
#define REQUEST_CODE_GL_GEN_LISTS 237
#define REQUEST_CODE_GL_GEN_PROGRAMS_ARB 238
#define REQUEST_CODE_GL_GEN_QUERIES 239
#define REQUEST_CODE_GL_GEN_RENDERBUFFERS 240
#define REQUEST_CODE_GL_GEN_SAMPLERS 241
#define REQUEST_CODE_GL_GEN_TEXTURES 242
#define REQUEST_CODE_GL_GEN_VERTEX_ARRAYS 243
#define REQUEST_CODE_GL_GENERATE_MIPMAP 244
#define REQUEST_CODE_GL_GET_ACTIVE_ATTRIB 245
#define REQUEST_CODE_GL_GET_ACTIVE_UNIFORM 246
#define REQUEST_CODE_GL_GET_ACTIVE_UNIFORM_BLOCK_NAME 247
#define REQUEST_CODE_GL_GET_ACTIVE_UNIFORM_BLOCKIV 248
#define REQUEST_CODE_GL_GET_ACTIVE_UNIFORM_NAME 249
#define REQUEST_CODE_GL_GET_ACTIVE_UNIFORMSIV 250
#define REQUEST_CODE_GL_GET_ATTACHED_SHADERS 251
#define REQUEST_CODE_GL_GET_ATTRIB_LOCATION 252
#define REQUEST_CODE_GL_GET_BOOLEANI_V 253
#define REQUEST_CODE_GL_GET_BOOLEANV 254
#define REQUEST_CODE_GL_GET_CLIP_PLANE 255
#define REQUEST_CODE_GL_GET_COMPRESSED_TEX_IMAGE 256
#define REQUEST_CODE_GL_GET_ERROR 257
#define REQUEST_CODE_GL_GET_FLOATV 258
#define REQUEST_CODE_GL_GET_FRAG_DATA_INDEX 259
#define REQUEST_CODE_GL_GET_FRAG_DATA_LOCATION 260
#define REQUEST_CODE_GL_GET_FRAMEBUFFER_ATTACHMENT_PARAMETERIV 261
#define REQUEST_CODE_GL_GET_HANDLE_ARB 262
#define REQUEST_CODE_GL_GET_INFO_LOG_ARB 263
#define REQUEST_CODE_GL_GET_INTEGER64I_V 264
#define REQUEST_CODE_GL_GET_INTEGER64V 265
#define REQUEST_CODE_GL_GET_INTEGERI_V 266
#define REQUEST_CODE_GL_GET_INTEGERV 267
#define REQUEST_CODE_GL_GET_INTERNALFORMATIV 268
#define REQUEST_CODE_GL_GET_LIGHTFV 269
#define REQUEST_CODE_GL_GET_LIGHTIV 270
#define REQUEST_CODE_GL_GET_MAPDV 271
#define REQUEST_CODE_GL_GET_MAPFV 272
#define REQUEST_CODE_GL_GET_MAPIV 273
#define REQUEST_CODE_GL_GET_MATERIALFV 274
#define REQUEST_CODE_GL_GET_MATERIALIV 275
#define REQUEST_CODE_GL_GET_MULTISAMPLEFV 276
#define REQUEST_CODE_GL_GET_OBJECT_PARAMETERFV_ARB 277
#define REQUEST_CODE_GL_GET_OBJECT_PARAMETERIV_ARB 278
#define REQUEST_CODE_GL_GET_PIXEL_MAPFV 279
#define REQUEST_CODE_GL_GET_PIXEL_MAPUIV 280
#define REQUEST_CODE_GL_GET_PIXEL_MAPUSV 281
#define REQUEST_CODE_GL_GET_POINTERV 282
#define REQUEST_CODE_GL_GET_POLYGON_STIPPLE 283
#define REQUEST_CODE_GL_GET_PROGRAM_ENV_PARAMETERDV_ARB 284
#define REQUEST_CODE_GL_GET_PROGRAM_ENV_PARAMETERFV_ARB 285
#define REQUEST_CODE_GL_GET_PROGRAM_INFO_LOG 286
#define REQUEST_CODE_GL_GET_PROGRAM_LOCAL_PARAMETERDV_ARB 287
#define REQUEST_CODE_GL_GET_PROGRAM_LOCAL_PARAMETERFV_ARB 288
#define REQUEST_CODE_GL_GET_PROGRAM_STRING_ARB 289
#define REQUEST_CODE_GL_GET_PROGRAMIV 290
#define REQUEST_CODE_GL_GET_QUERY_OBJECTUIV 291
#define REQUEST_CODE_GL_GET_QUERYIV 292
#define REQUEST_CODE_GL_GET_RENDERBUFFER_PARAMETERIV 293
#define REQUEST_CODE_GL_GET_SAMPLER_PARAMETERFV 294
#define REQUEST_CODE_GL_GET_SHADER_INFO_LOG 295
#define REQUEST_CODE_GL_GET_SHADER_PRECISION_FORMAT 296
#define REQUEST_CODE_GL_GET_SHADER_SOURCE 297
#define REQUEST_CODE_GL_GET_SHADERIV 298
#define REQUEST_CODE_GL_GET_STRING 299
#define REQUEST_CODE_GL_GET_SYNCIV 300
#define REQUEST_CODE_GL_GET_TEX_ENVFV 301
#define REQUEST_CODE_GL_GET_TEX_ENVIV 302
#define REQUEST_CODE_GL_GET_TEX_GENDV 303
#define REQUEST_CODE_GL_GET_TEX_GENFV 304
#define REQUEST_CODE_GL_GET_TEX_GENIV 305
#define REQUEST_CODE_GL_GET_TEX_IMAGE 306
#define REQUEST_CODE_GL_GET_TEX_LEVEL_PARAMETERFV 307
#define REQUEST_CODE_GL_GET_TEX_PARAMETERFV 308
#define REQUEST_CODE_GL_GET_TRANSFORM_FEEDBACK_VARYING 309
#define REQUEST_CODE_GL_GET_UNIFORM_BLOCK_INDEX 310
#define REQUEST_CODE_GL_GET_UNIFORM_INDICES 311
#define REQUEST_CODE_GL_GET_UNIFORM_LOCATION 312
#define REQUEST_CODE_GL_GET_UNIFORMFV 313
#define REQUEST_CODE_GL_GET_UNIFORMIV 314
#define REQUEST_CODE_GL_GET_UNIFORMUIV 315
#define REQUEST_CODE_GL_GET_VERTEX_ATTRIB_IIV 316
#define REQUEST_CODE_GL_GET_VERTEX_ATTRIB_IUIV 317
#define REQUEST_CODE_GL_GET_VERTEX_ATTRIB_POINTERV 318
#define REQUEST_CODE_GL_GET_VERTEX_ATTRIBDV 319
#define REQUEST_CODE_GL_GET_VERTEX_ATTRIBFV 320
#define REQUEST_CODE_GL_GET_VERTEX_ATTRIBIV 321
#define REQUEST_CODE_GL_HINT 322
#define REQUEST_CODE_GL_INDEX_MASK 323
#define REQUEST_CODE_GL_INDEX_POINTER 324
#define REQUEST_CODE_GL_INDEXF 325
#define REQUEST_CODE_GL_INIT_NAMES 326
#define REQUEST_CODE_GL_INTERLEAVED_ARRAYS 327
#define REQUEST_CODE_GL_IS_BUFFER 328
#define REQUEST_CODE_GL_IS_ENABLED 329
#define REQUEST_CODE_GL_IS_ENABLEDI 330
#define REQUEST_CODE_GL_IS_FRAMEBUFFER 331
#define REQUEST_CODE_GL_IS_LIST 332
#define REQUEST_CODE_GL_IS_PROGRAM 333
#define REQUEST_CODE_GL_IS_QUERY 334
#define REQUEST_CODE_GL_IS_RENDERBUFFER 335
#define REQUEST_CODE_GL_IS_SAMPLER 336
#define REQUEST_CODE_GL_IS_SHADER 337
#define REQUEST_CODE_GL_IS_SYNC 338
#define REQUEST_CODE_GL_IS_TEXTURE 339
#define REQUEST_CODE_GL_IS_VERTEX_ARRAY 340
#define REQUEST_CODE_GL_LIGHT_MODELF 341
#define REQUEST_CODE_GL_LIGHT_MODELFV 342
#define REQUEST_CODE_GL_LIGHT_MODELI 343
#define REQUEST_CODE_GL_LIGHT_MODELIV 344
#define REQUEST_CODE_GL_LIGHTF 345
#define REQUEST_CODE_GL_LIGHTFV 346
#define REQUEST_CODE_GL_LINE_STIPPLE 347
#define REQUEST_CODE_GL_LINE_WIDTH 348
#define REQUEST_CODE_GL_LINK_PROGRAM 349
#define REQUEST_CODE_GL_LIST_BASE 350
#define REQUEST_CODE_GL_LOAD_IDENTITY 351
#define REQUEST_CODE_GL_LOAD_MATRIXF 352
#define REQUEST_CODE_GL_LOAD_NAME 353
#define REQUEST_CODE_GL_LOAD_TRANSPOSE_MATRIXF 354
#define REQUEST_CODE_GL_LOGIC_OP 355
#define REQUEST_CODE_GL_MAP1D 356
#define REQUEST_CODE_GL_MAP1F 357
#define REQUEST_CODE_GL_MAP2D 358
#define REQUEST_CODE_GL_MAP2F 359
#define REQUEST_CODE_GL_MAP_GRID1D 360
#define REQUEST_CODE_GL_MAP_GRID1F 361
#define REQUEST_CODE_GL_MAP_GRID2D 362
#define REQUEST_CODE_GL_MAP_GRID2F 363
#define REQUEST_CODE_GL_MATERIALF 364
#define REQUEST_CODE_GL_MATERIALFV 365
#define REQUEST_CODE_GL_MATRIX_MODE 366
#define REQUEST_CODE_GL_MIN_SAMPLE_SHADING 367
#define REQUEST_CODE_GL_MULT_MATRIXF 368
#define REQUEST_CODE_GL_MULT_TRANSPOSE_MATRIXF 369
#define REQUEST_CODE_GL_MULTI_DRAW_ELEMENTS_BASE_VERTEX 370
#define REQUEST_CODE_GL_MULTI_TEX_COORD4F 371
#define REQUEST_CODE_GL_NEW_LIST 372
#define REQUEST_CODE_GL_NORMAL3F 373
#define REQUEST_CODE_GL_NORMAL_POINTER 374
#define REQUEST_CODE_GL_ORTHO 375
#define REQUEST_CODE_GL_PASS_THROUGH 376
#define REQUEST_CODE_GL_PATCH_PARAMETERFV 377
#define REQUEST_CODE_GL_PATCH_PARAMETERI 378
#define REQUEST_CODE_GL_PIXEL_MAPFV 379
#define REQUEST_CODE_GL_PIXEL_MAPUIV 380
#define REQUEST_CODE_GL_PIXEL_MAPUSV 381
#define REQUEST_CODE_GL_PIXEL_STOREI 382
#define REQUEST_CODE_GL_PIXEL_TRANSFERF 383
#define REQUEST_CODE_GL_PIXEL_TRANSFERI 384
#define REQUEST_CODE_GL_PIXEL_ZOOM 385
#define REQUEST_CODE_GL_POINT_PARAMETERF 386
#define REQUEST_CODE_GL_POINT_PARAMETERFV 387
#define REQUEST_CODE_GL_POINT_PARAMETERI 388
#define REQUEST_CODE_GL_POINT_PARAMETERIV 389
#define REQUEST_CODE_GL_POINT_SIZE 390
#define REQUEST_CODE_GL_POLYGON_MODE 391
#define REQUEST_CODE_GL_POLYGON_OFFSET 392
#define REQUEST_CODE_GL_POLYGON_STIPPLE 393
#define REQUEST_CODE_GL_POP_ATTRIB 394
#define REQUEST_CODE_GL_POP_CLIENT_ATTRIB 395
#define REQUEST_CODE_GL_POP_MATRIX 396
#define REQUEST_CODE_GL_POP_NAME 397
#define REQUEST_CODE_GL_PRIORITIZE_TEXTURES 398
#define REQUEST_CODE_GL_PROGRAM_ENV_PARAMETER4F_ARB 399
#define REQUEST_CODE_GL_PROGRAM_LOCAL_PARAMETER4F_ARB 400
#define REQUEST_CODE_GL_PROGRAM_STRING_ARB 401
#define REQUEST_CODE_GL_PROVOKING_VERTEX 402
#define REQUEST_CODE_GL_PUSH_ATTRIB 403
#define REQUEST_CODE_GL_PUSH_CLIENT_ATTRIB 404
#define REQUEST_CODE_GL_PUSH_MATRIX 405
#define REQUEST_CODE_GL_PUSH_NAME 406
#define REQUEST_CODE_GL_QUERY_COUNTER 407
#define REQUEST_CODE_GL_RASTER_POS4F 408
#define REQUEST_CODE_GL_READ_BUFFER 409
#define REQUEST_CODE_GL_READ_PIXELS 410
#define REQUEST_CODE_GL_RELEASE_SHADER_COMPILER 411
#define REQUEST_CODE_GL_RENDER_MODE 412
#define REQUEST_CODE_GL_RENDERBUFFER_STORAGE 413
#define REQUEST_CODE_GL_RENDERBUFFER_STORAGE_MULTISAMPLE 414
#define REQUEST_CODE_GL_ROTATEF 415
#define REQUEST_CODE_GL_SAMPLE_COVERAGE 416
#define REQUEST_CODE_GL_SAMPLE_MASKI 417
#define REQUEST_CODE_GL_SAMPLER_PARAMETERF 418
#define REQUEST_CODE_GL_SAMPLER_PARAMETERFV 419
#define REQUEST_CODE_GL_SCALEF 420
#define REQUEST_CODE_GL_SCISSOR 421
#define REQUEST_CODE_GL_SECONDARY_COLOR3F 422
#define REQUEST_CODE_GL_SECONDARY_COLOR_POINTER 423
#define REQUEST_CODE_GL_SELECT_BUFFER 424
#define REQUEST_CODE_GL_SHADE_MODEL 425
#define REQUEST_CODE_GL_SHADER_BINARY 426
#define REQUEST_CODE_GL_SHADER_SOURCE 427
#define REQUEST_CODE_GL_STENCIL_FUNC 428
#define REQUEST_CODE_GL_STENCIL_FUNC_SEPARATE 429
#define REQUEST_CODE_GL_STENCIL_MASK 430
#define REQUEST_CODE_GL_STENCIL_MASK_SEPARATE 431
#define REQUEST_CODE_GL_STENCIL_OP 432
#define REQUEST_CODE_GL_STENCIL_OP_SEPARATE 433
#define REQUEST_CODE_GL_TEX_BUFFER 434
#define REQUEST_CODE_GL_TEX_COORD4F 435
#define REQUEST_CODE_GL_TEX_COORD_POINTER 436
#define REQUEST_CODE_GL_TEX_ENVF 437
#define REQUEST_CODE_GL_TEX_ENVFV 438
#define REQUEST_CODE_GL_TEX_GEND 439
#define REQUEST_CODE_GL_TEX_GENDV 440
#define REQUEST_CODE_GL_TEX_GENF 441
#define REQUEST_CODE_GL_TEX_GENFV 442
#define REQUEST_CODE_GL_TEX_GENI 443
#define REQUEST_CODE_GL_TEX_GENIV 444
#define REQUEST_CODE_GL_TEX_IMAGE2D 445
#define REQUEST_CODE_GL_TEX_IMAGE2DMULTISAMPLE 446
#define REQUEST_CODE_GL_TEX_IMAGE3D 447
#define REQUEST_CODE_GL_TEX_IMAGE3DMULTISAMPLE 448
#define REQUEST_CODE_GL_TEX_PARAMETERF 449
#define REQUEST_CODE_GL_TEX_PARAMETERFV 450
#define REQUEST_CODE_GL_TEX_SUB_IMAGE2D 451
#define REQUEST_CODE_GL_TEX_SUB_IMAGE3D 452
#define REQUEST_CODE_GL_TRANSFORM_FEEDBACK_VARYINGS 453
#define REQUEST_CODE_GL_TRANSLATEF 454
#define REQUEST_CODE_GL_UNIFORM1F 455
#define REQUEST_CODE_GL_UNIFORM1FV 456
#define REQUEST_CODE_GL_UNIFORM1I 457
#define REQUEST_CODE_GL_UNIFORM1IV 458
#define REQUEST_CODE_GL_UNIFORM1UI 459
#define REQUEST_CODE_GL_UNIFORM1UIV 460
#define REQUEST_CODE_GL_UNIFORM2F 461
#define REQUEST_CODE_GL_UNIFORM2FV 462
#define REQUEST_CODE_GL_UNIFORM2I 463
#define REQUEST_CODE_GL_UNIFORM2IV 464
#define REQUEST_CODE_GL_UNIFORM2UI 465
#define REQUEST_CODE_GL_UNIFORM2UIV 466
#define REQUEST_CODE_GL_UNIFORM3F 467
#define REQUEST_CODE_GL_UNIFORM3FV 468
#define REQUEST_CODE_GL_UNIFORM3I 469
#define REQUEST_CODE_GL_UNIFORM3IV 470
#define REQUEST_CODE_GL_UNIFORM3UI 471
#define REQUEST_CODE_GL_UNIFORM3UIV 472
#define REQUEST_CODE_GL_UNIFORM4F 473
#define REQUEST_CODE_GL_UNIFORM4FV 474
#define REQUEST_CODE_GL_UNIFORM4I 475
#define REQUEST_CODE_GL_UNIFORM4IV 476
#define REQUEST_CODE_GL_UNIFORM4UI 477
#define REQUEST_CODE_GL_UNIFORM4UIV 478
#define REQUEST_CODE_GL_UNIFORM_BLOCK_BINDING 479
#define REQUEST_CODE_GL_UNIFORM_MATRIX2FV 480
#define REQUEST_CODE_GL_UNIFORM_MATRIX2X3FV 481
#define REQUEST_CODE_GL_UNIFORM_MATRIX2X4FV 482
#define REQUEST_CODE_GL_UNIFORM_MATRIX3FV 483
#define REQUEST_CODE_GL_UNIFORM_MATRIX3X2FV 484
#define REQUEST_CODE_GL_UNIFORM_MATRIX3X4FV 485
#define REQUEST_CODE_GL_UNIFORM_MATRIX4FV 486
#define REQUEST_CODE_GL_UNIFORM_MATRIX4X2FV 487
#define REQUEST_CODE_GL_UNIFORM_MATRIX4X3FV 488
#define REQUEST_CODE_GL_USE_PROGRAM 489
#define REQUEST_CODE_GL_VALIDATE_PROGRAM 490
#define REQUEST_CODE_GL_VERTEX4F 491
#define REQUEST_CODE_GL_VERTEX_ATTRIB1F 492
#define REQUEST_CODE_GL_VERTEX_ATTRIB2F 493
#define REQUEST_CODE_GL_VERTEX_ATTRIB3F 494
#define REQUEST_CODE_GL_VERTEX_ATTRIB4NBV 495
#define REQUEST_CODE_GL_VERTEX_ATTRIB4NIV 496
#define REQUEST_CODE_GL_VERTEX_ATTRIB4NSV 497
#define REQUEST_CODE_GL_VERTEX_ATTRIB4NUB 498
#define REQUEST_CODE_GL_VERTEX_ATTRIB4NUBV 499
#define REQUEST_CODE_GL_VERTEX_ATTRIB4NUIV 500
#define REQUEST_CODE_GL_VERTEX_ATTRIB4NUSV 501
#define REQUEST_CODE_GL_VERTEX_ATTRIB4F 502
#define REQUEST_CODE_GL_VERTEX_ATTRIB_DIVISOR 503
#define REQUEST_CODE_GL_VERTEX_ATTRIB_I1I 504
#define REQUEST_CODE_GL_VERTEX_ATTRIB_I1IV 505
#define REQUEST_CODE_GL_VERTEX_ATTRIB_I1UI 506
#define REQUEST_CODE_GL_VERTEX_ATTRIB_I1UIV 507
#define REQUEST_CODE_GL_VERTEX_ATTRIB_I2I 508
#define REQUEST_CODE_GL_VERTEX_ATTRIB_I2IV 509
#define REQUEST_CODE_GL_VERTEX_ATTRIB_I2UI 510
#define REQUEST_CODE_GL_VERTEX_ATTRIB_I2UIV 511
#define REQUEST_CODE_GL_VERTEX_ATTRIB_I3I 512
#define REQUEST_CODE_GL_VERTEX_ATTRIB_I3IV 513
#define REQUEST_CODE_GL_VERTEX_ATTRIB_I3UI 514
#define REQUEST_CODE_GL_VERTEX_ATTRIB_I3UIV 515
#define REQUEST_CODE_GL_VERTEX_ATTRIB_I4BV 516
#define REQUEST_CODE_GL_VERTEX_ATTRIB_I4I 517
#define REQUEST_CODE_GL_VERTEX_ATTRIB_I4IV 518
#define REQUEST_CODE_GL_VERTEX_ATTRIB_I4SV 519
#define REQUEST_CODE_GL_VERTEX_ATTRIB_I4UBV 520
#define REQUEST_CODE_GL_VERTEX_ATTRIB_I4UI 521
#define REQUEST_CODE_GL_VERTEX_ATTRIB_I4UIV 522
#define REQUEST_CODE_GL_VERTEX_ATTRIB_I4USV 523
#define REQUEST_CODE_GL_VERTEX_ATTRIB_IPOINTER 524
#define REQUEST_CODE_GL_VERTEX_ATTRIB_POINTER 525
#define REQUEST_CODE_GL_VERTEX_POINTER 526
#define REQUEST_CODE_GL_VIEWPORT 527
#define REQUEST_CODE_GL_WAIT_SYNC 528
#define REQUEST_CODE_GL_WINDOW_POS3F 529

#if DEBUG_MODE
static const char* requestCodeStrings[] = {
    "glAccum",
    "glActiveTexture",
    "glAlphaFunc",
    "glArrayElement",
    "glAttachShader",
    "glBegin",
    "glBeginConditionalRender",
    "glBeginQuery",
    "glBeginTransformFeedback",
    "glBindAttribLocation",
    "glBindBuffer",
    "glBindBufferBase",
    "glBindBufferRange",
    "glBindFragDataLocation",
    "glBindFragDataLocationIndexed",
    "glBindFramebuffer",
    "glBindProgramARB",
    "glBindRenderbuffer",
    "glBindSampler",
    "glBindTexture",
    "glBindVertexArray",
    "glBitmap",
    "glBlendColor",
    "glBlendEquation",
    "glBlendEquationSeparate",
    "glBlendEquationSeparatei",
    "glBlendEquationi",
    "glBlendFunc",
    "glBlendFuncSeparate",
    "glBlendFuncSeparatei",
    "glBlendFunci",
    "glBlitFramebuffer",
    "glBufferData",
    "glBufferStorage",
    "glBufferSubData",
    "glCallList",
    "glCallLists",
    "glCheckFramebufferStatus",
    "glClampColor",
    "glClear",
    "glClearAccum",
    "glClearBufferfi",
    "glClearBufferfv",
    "glClearBufferiv",
    "glClearBufferuiv",
    "glClearColor",
    "glClearDepthf",
    "glClearIndex",
    "glClearStencil",
    "glClientActiveTexture",
    "glClientWaitSync",
    "glClipPlane",
    "glColor4f",
    "glColorMask",
    "glColorMaski",
    "glColorMaterial",
    "glColorPointer",
    "glCompileShader",
    "glCompressedTexImage2D",
    "glCompressedTexSubImage2D",
    "glCopyBufferSubData",
    "glCopyPixels",
    "glCopyTexImage2D",
    "glCopyTexSubImage2D",
    "glCopyTexSubImage3D",
    "glCreateProgram",
    "glCreateShader",
    "glCullFace",
    "glDeleteBuffers",
    "glDeleteFramebuffers",
    "glDeleteLists",
    "glDeleteObjectARB",
    "glDeleteProgram",
    "glDeleteProgramsARB",
    "glDeleteQueries",
    "glDeleteRenderbuffers",
    "glDeleteSamplers",
    "glDeleteShader",
    "glDeleteSync",
    "glDeleteTextures",
    "glDeleteVertexArrays",
    "glDepthFunc",
    "glDepthMask",
    "glDepthRangef",
    "glDetachShader",
    "glDisable",
    "glDisableClientState",
    "glDisableVertexAttribArray",
    "glDisablei",
    "glDispatchCompute",
    "glDispatchComputeIndirect",
    "glDrawArrays",
    "glDrawArraysInstanced",
    "glDrawBuffer",
    "glDrawBuffers",
    "glDrawElements",
    "glDrawElementsBaseVertex",
    "glDrawElementsInstanced",
    "glDrawElementsInstancedBaseVertex",
    "glDrawPixels",
    "glDrawRangeElements",
    "glDrawRangeElementsBaseVertex",
    "glEdgeFlag",
    "glEdgeFlagPointer",
    "glEdgeFlagv",
    "glEnable",
    "glEnableClientState",
    "glEnableVertexAttribArray",
    "glEnablei",
    "glEnd",
    "glEndConditionalRender",
    "glEndList",
    "glEndQuery",
    "glEndTransformFeedback",
    "glEvalCoord2f",
    "glEvalMesh1",
    "glEvalMesh2",
    "glEvalPoint1",
    "glEvalPoint2",
    "glFeedbackBuffer",
    "glFenceSync",
    "glFinish",
    "glFlush",
    "glFlushMappedBufferRange",
    "glFogCoordPointer",
    "glFogCoordf",
    "glFogf",
    "glFogfv",
    "glFramebufferRenderbuffer",
    "glFramebufferTexture",
    "glFramebufferTexture2D",
    "glFramebufferTexture3D",
    "glFramebufferTextureLayer",
    "glFrontFace",
    "glFrustum",
    "glGenBuffers",
    "glGenFramebuffers",
    "glGenLists",
    "glGenProgramsARB",
    "glGenQueries",
    "glGenRenderbuffers",
    "glGenSamplers",
    "glGenTextures",
    "glGenVertexArrays",
    "glGenerateMipmap",
    "glGetActiveAttrib",
    "glGetActiveUniform",
    "glGetActiveUniformBlockName",
    "glGetActiveUniformBlockiv",
    "glGetActiveUniformName",
    "glGetActiveUniformsiv",
    "glGetAttachedShaders",
    "glGetAttribLocation",
    "glGetBooleani_v",
    "glGetBooleanv",
    "glGetClipPlane",
    "glGetCompressedTexImage",
    "glGetError",
    "glGetFloatv",
    "glGetFragDataIndex",
    "glGetFragDataLocation",
    "glGetFramebufferAttachmentParameteriv",
    "glGetHandleARB",
    "glGetInfoLogARB",
    "glGetInteger64i_v",
    "glGetInteger64v",
    "glGetIntegeri_v",
    "glGetIntegerv",
    "glGetInternalformativ",
    "glGetLightfv",
    "glGetLightiv",
    "glGetMapdv",
    "glGetMapfv",
    "glGetMapiv",
    "glGetMaterialfv",
    "glGetMaterialiv",
    "glGetMultisamplefv",
    "glGetObjectParameterfvARB",
    "glGetObjectParameterivARB",
    "glGetPixelMapfv",
    "glGetPixelMapuiv",
    "glGetPixelMapusv",
    "glGetPointerv",
    "glGetPolygonStipple",
    "glGetProgramEnvParameterdvARB",
    "glGetProgramEnvParameterfvARB",
    "glGetProgramInfoLog",
    "glGetProgramLocalParameterdvARB",
    "glGetProgramLocalParameterfvARB",
    "glGetProgramStringARB",
    "glGetProgramiv",
    "glGetQueryObjectuiv",
    "glGetQueryiv",
    "glGetRenderbufferParameteriv",
    "glGetSamplerParameterfv",
    "glGetShaderInfoLog",
    "glGetShaderPrecisionFormat",
    "glGetShaderSource",
    "glGetShaderiv",
    "glGetString",
    "glGetSynciv",
    "glGetTexEnvfv",
    "glGetTexEnviv",
    "glGetTexGendv",
    "glGetTexGenfv",
    "glGetTexGeniv",
    "glGetTexImage",
    "glGetTexLevelParameterfv",
    "glGetTexParameterfv",
    "glGetTransformFeedbackVarying",
    "glGetUniformBlockIndex",
    "glGetUniformIndices",
    "glGetUniformLocation",
    "glGetUniformfv",
    "glGetUniformiv",
    "glGetUniformuiv",
    "glGetVertexAttribIiv",
    "glGetVertexAttribIuiv",
    "glGetVertexAttribPointerv",
    "glGetVertexAttribdv",
    "glGetVertexAttribfv",
    "glGetVertexAttribiv",
    "glHint",
    "glIndexMask",
    "glIndexPointer",
    "glIndexf",
    "glInitNames",
    "glInterleavedArrays",
    "glIsBuffer",
    "glIsEnabled",
    "glIsEnabledi",
    "glIsFramebuffer",
    "glIsList",
    "glIsProgram",
    "glIsQuery",
    "glIsRenderbuffer",
    "glIsSampler",
    "glIsShader",
    "glIsSync",
    "glIsTexture",
    "glIsVertexArray",
    "glLightModelf",
    "glLightModelfv",
    "glLightModeli",
    "glLightModeliv",
    "glLightf",
    "glLightfv",
    "glLineStipple",
    "glLineWidth",
    "glLinkProgram",
    "glListBase",
    "glLoadIdentity",
    "glLoadMatrixf",
    "glLoadName",
    "glLoadTransposeMatrixf",
    "glLogicOp",
    "glMap1d",
    "glMap1f",
    "glMap2d",
    "glMap2f",
    "glMapGrid1d",
    "glMapGrid1f",
    "glMapGrid2d",
    "glMapGrid2f",
    "glMaterialf",
    "glMaterialfv",
    "glMatrixMode",
    "glMinSampleShading",
    "glMultMatrixf",
    "glMultTransposeMatrixf",
    "glMultiDrawElementsBaseVertex",
    "glMultiTexCoord4f",
    "glNewList",
    "glNormal3f",
    "glNormalPointer",
    "glOrtho",
    "glPassThrough",
    "glPatchParameterfv",
    "glPatchParameteri",
    "glPixelMapfv",
    "glPixelMapuiv",
    "glPixelMapusv",
    "glPixelStorei",
    "glPixelTransferf",
    "glPixelTransferi",
    "glPixelZoom",
    "glPointParameterf",
    "glPointParameterfv",
    "glPointParameteri",
    "glPointParameteriv",
    "glPointSize",
    "glPolygonMode",
    "glPolygonOffset",
    "glPolygonStipple",
    "glPopAttrib",
    "glPopClientAttrib",
    "glPopMatrix",
    "glPopName",
    "glPrioritizeTextures",
    "glProgramEnvParameter4fARB",
    "glProgramLocalParameter4fARB",
    "glProgramStringARB",
    "glProvokingVertex",
    "glPushAttrib",
    "glPushClientAttrib",
    "glPushMatrix",
    "glPushName",
    "glQueryCounter",
    "glRasterPos4f",
    "glReadBuffer",
    "glReadPixels",
    "glReleaseShaderCompiler",
    "glRenderMode",
    "glRenderbufferStorage",
    "glRenderbufferStorageMultisample",
    "glRotatef",
    "glSampleCoverage",
    "glSampleMaski",
    "glSamplerParameterf",
    "glSamplerParameterfv",
    "glScalef",
    "glScissor",
    "glSecondaryColor3f",
    "glSecondaryColorPointer",
    "glSelectBuffer",
    "glShadeModel",
    "glShaderBinary",
    "glShaderSource",
    "glStencilFunc",
    "glStencilFuncSeparate",
    "glStencilMask",
    "glStencilMaskSeparate",
    "glStencilOp",
    "glStencilOpSeparate",
    "glTexBuffer",
    "glTexCoord4f",
    "glTexCoordPointer",
    "glTexEnvf",
    "glTexEnvfv",
    "glTexGend",
    "glTexGendv",
    "glTexGenf",
    "glTexGenfv",
    "glTexGeni",
    "glTexGeniv",
    "glTexImage2D",
    "glTexImage2DMultisample",
    "glTexImage3D",
    "glTexImage3DMultisample",
    "glTexParameterf",
    "glTexParameterfv",
    "glTexSubImage2D",
    "glTexSubImage3D",
    "glTransformFeedbackVaryings",
    "glTranslatef",
    "glUniform1f",
    "glUniform1fv",
    "glUniform1i",
    "glUniform1iv",
    "glUniform1ui",
    "glUniform1uiv",
    "glUniform2f",
    "glUniform2fv",
    "glUniform2i",
    "glUniform2iv",
    "glUniform2ui",
    "glUniform2uiv",
    "glUniform3f",
    "glUniform3fv",
    "glUniform3i",
    "glUniform3iv",
    "glUniform3ui",
    "glUniform3uiv",
    "glUniform4f",
    "glUniform4fv",
    "glUniform4i",
    "glUniform4iv",
    "glUniform4ui",
    "glUniform4uiv",
    "glUniformBlockBinding",
    "glUniformMatrix2fv",
    "glUniformMatrix2x3fv",
    "glUniformMatrix2x4fv",
    "glUniformMatrix3fv",
    "glUniformMatrix3x2fv",
    "glUniformMatrix3x4fv",
    "glUniformMatrix4fv",
    "glUniformMatrix4x2fv",
    "glUniformMatrix4x3fv",
    "glUseProgram",
    "glValidateProgram",
    "glVertex4f",
    "glVertexAttrib1f",
    "glVertexAttrib2f",
    "glVertexAttrib3f",
    "glVertexAttrib4Nbv",
    "glVertexAttrib4Niv",
    "glVertexAttrib4Nsv",
    "glVertexAttrib4Nub",
    "glVertexAttrib4Nubv",
    "glVertexAttrib4Nuiv",
    "glVertexAttrib4Nusv",
    "glVertexAttrib4f",
    "glVertexAttribDivisor",
    "glVertexAttribI1i",
    "glVertexAttribI1iv",
    "glVertexAttribI1ui",
    "glVertexAttribI1uiv",
    "glVertexAttribI2i",
    "glVertexAttribI2iv",
    "glVertexAttribI2ui",
    "glVertexAttribI2uiv",
    "glVertexAttribI3i",
    "glVertexAttribI3iv",
    "glVertexAttribI3ui",
    "glVertexAttribI3uiv",
    "glVertexAttribI4bv",
    "glVertexAttribI4i",
    "glVertexAttribI4iv",
    "glVertexAttribI4sv",
    "glVertexAttribI4ubv",
    "glVertexAttribI4ui",
    "glVertexAttribI4uiv",
    "glVertexAttribI4usv",
    "glVertexAttribIPointer",
    "glVertexAttribPointer",
    "glVertexPointer",
    "glViewport",
    "glWaitSync",
    "glWindowPos3f",
};

static inline const char* requestCodeToString(short requestCode) {
    int index = requestCode - REQUEST_CODE_GL_CALL_START;
    return index >= 0 && index < REQUEST_CODE_GL_CALL_COUNT ? requestCodeStrings[index] : "Unknown";
}
#endif

#endif