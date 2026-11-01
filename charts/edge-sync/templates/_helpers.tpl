{{- define "edge-sync.name" -}}
{{- default .Chart.Name .Values.nameOverride | trunc 63 | trimSuffix "-" -}}
{{- end -}}

{{- define "edge-sync.fullname" -}}
{{- printf "%s-%s" .Release.Name (include "edge-sync.name" .) | trunc 63 | trimSuffix "-" -}}
{{- end -}}

{{- define "edge-sync.labels" -}}
app.kubernetes.io/name: {{ include "edge-sync.name" . }}
app.kubernetes.io/instance: {{ .Release.Name }}
app.kubernetes.io/managed-by: {{ .Release.Service }}
{{- end -}}
