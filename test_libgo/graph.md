flowchart TB
    %% Input
    A[Natural Language Description]

    %% Encoders
    A --> B1[CodeBERT Encoder<br/>(Bidirectional Semantic Encoder)]
    A --> B2[CodeT5 Encoder<br/>(Generative Encoder)]

    %% Encoder Outputs
    B1 -->|Token-level Semantic Representations| H_b[H_b]
    B2 -->|Generation-oriented Representations| H_t[H_t]

    %% Fusion Layer
    subgraph Fusion["Cross-Attention Semantic Fusion Layer (Proposed)"]
        direction TB
        H_t --> Q[Query: H_t]
        H_b --> K[Key: H_b]
        H_b --> V[Value: H_b]
        Q --> CA[Cross-Attention]
        K --> CA
        V --> CA
        CA --> H_attn[Aligned Semantic Features]
        H_t --> Concat
        H_attn --> Concat
        Concat --> FFN[Feed-Forward Network + LayerNorm]
        FFN --> H_fused[H_fused]
    end

    %% Decoder
    H_fused --> D[CodeT5 Decoder<br/>(Autoregressive Generation)]

    %% Output
    D --> O[Generated Code Sequence]

    %% Training Notes
    B1 -. Frozen during training .- Fusion
    B2 -. Frozen in early stage .- Fusion